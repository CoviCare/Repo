#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#define USE_ARDUINO_INTERRUPTS false
#include <PulseSensorPlayground.h>
#include <Adafruit_MLX90614.h>
#include <Wire.h>

// Watson IoT connection details
#define MQTT_HOST "ngib7e.messaging.internetofthings.ibmcloud.com"
#define MQTT_PORT 1883
#define MQTT_DEVICEID "d:ngib7e:ESP8266:dev0nm1"
#define MQTT_USER "use-token-auth"
#define MQTT_TOKEN "KMBSfL6DDNSSf?q+Rc"
#define MQTT_TOPIC "iot-2/evt/status/fmt/json"
#define MQTT_TOPIC_DISPLAY "iot-2/cmd/display/fmt/json"

// Add WiFi connection information
char ssid[] = "anjana";     //  your network SSID (name)
char pass[] = "mdl17ec016";  // your network password

Adafruit_MLX90614 mlx = Adafruit_MLX90614();
PulseSensorPlayground pulseSensor;


const int PULSE_INPUT = A0;
const int THRESHOLD = 550;   // Adjust this number to avoid noise when idle
float temp_celsius=0;
float temp_fahrenheit = 0;
int myBPM; 

// MQTT objects
void callback(char* topic, byte* payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, callback, wifiClient);

// variables to hold data
StaticJsonDocument<100> jsonDoc;
JsonObject payload = jsonDoc.to<JsonObject>();
JsonObject status = payload.createNestedObject("d");
static char msg[100];

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] : ");
  
  payload[length] = 0; // ensure valid content is zero terminated so can treat as c-string
  Serial.println((char *)payload);
}

void setup() {
  
  Serial.begin(115200);
  Serial.setTimeout(2000);
  while (!Serial) { }
  Serial.println();
  Serial.println("ESP8266 Sensor Application");

  // Start WiFi connection
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi Connected");
  
  pulseSensor.analogInput(PULSE_INPUT);
  pulseSensor.setThreshold(THRESHOLD);
  mlx.begin();

  if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
    Serial.println("MQTT Connected");
    mqtt.subscribe(MQTT_TOPIC_DISPLAY);

  } else {
    Serial.println("MQTT Failed to connect!");
    ESP.reset();
  }
  
}
void loop() {
  mqtt.loop();
  while (!mqtt.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) {
      Serial.println("MQTT Connected");
      mqtt.subscribe(MQTT_TOPIC_DISPLAY);
      mqtt.loop();
    } else {
      Serial.println("MQTT Failed to connect!");
      delay(5000);
    }
  }
  long int reading = analogRead(0);
  myBPM=map(reading,0,1023,40,120);
  temp_celsius= mlx.readObjectTempC();
  temp_fahrenheit=mlx.readObjectTempF();
  myBPM = pulseSensor.getBeatsPerMinute();
  
  if(isnan(myBPM)||isnan(temp_celsius)){
    Serial.println("Failed to read from sensor!");
  } 
  else {  
            
            status["celsius"]=temp_celsius;
            status["fahrenheit"]=temp_fahrenheit;
            status["ecg"]=reading;
            if ((reading>=240)&&(reading<=550)){
            status["heart"]=0;
            }
            else if (pulseSensor.sawNewSample()) {
                pulseSensor.outputSample();
                if (pulseSensor.sawStartOfBeat()) 
            {
               status["heart"]=myBPM;
               delay(20);
        }}
     
}
    serializeJson(jsonDoc, msg, 100);
    Serial.println(msg);
    if (!mqtt.publish(MQTT_TOPIC, msg)) {
            Serial.println("MQTT Publish failed");
            }
    
             
  }


/* for (int i = 0; i < 10; i++) {
    mqtt.loop();
    delay(1000);
  }*/


