



#include <ESP8266WiFi.h>
#include <ArduinoJson.h>
#include <PubSubClient.h>
#include <TinyGPS++.h> 
#include <SoftwareSerial.h>


 TinyGPSPlus gps;  // The TinyGPS++ object

  SoftwareSerial ss(4, 5); // The serial connection to the GPS device

  /*
   * vcc--->3.3
   * gnd---> gnd
   * rx---->d1--->5
   * tx---->d2--->4
   * 
   * trigger---->13--->d7
   * echo------->12---->d6
   * vcc------>5v
   * gnd------>gnd
   * 
   * buzzer ----->d4
   * vcc---------->3.3v
   * 
   * 
   * 
   */
   
// Watson IoT connection details
#define MQTT_HOST "ngib7e.messaging.internetofthings.ibmcloud.com"
#define MQTT_PORT 1883
#define MQTT_DEVICEID "d:ngib7e:ESP8266:dev03"
#define MQTT_USER "use-token-auth"
#define MQTT_TOKEN "surya0484"
#define MQTT_TOPIC "iot-2/evt/status/fmt/json"
#define MQTT_TOPIC_DISPLAY "iot-2/cmd/display/fmt/json"

//----------- Enter you Wi-Fi Details---------//
char ssid[] = "Redmi";//"Airtel_Zerotouch"; //SSID
char pass[] = "aj000167";//"12345678"; // Password
//-------------------------------------------//
float latitude , longitude;
 String  lat_str , lng_str;
  int pm;
const int trigger = 13;
const int echo = 12;
long T;
float distanceCM;
//WiFiClient  client;
void callback(char* topic, byte* payload, unsigned int length);
WiFiClient wifiClient;
PubSubClient mqtt(MQTT_HOST, MQTT_PORT, callback, wifiClient);

StaticJsonDocument<300> jsonDoc;
JsonObject payload = jsonDoc.to<JsonObject>();
JsonObject status = payload.createNestedObject("d"); 
static char msg[150];

void callback(char* topic, byte* payload, unsigned int length) {
  // handle message arrived
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] : ");
  
  payload[length] = 0; // ensure valid content is zero terminated so can treat as c-string
  Serial.println((char *)payload);
}



void setup()
{
  
  Serial.begin(115200);
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
  pinMode(14,OUTPUT);
 
  WiFi.mode(WIFI_STA);
  //ThingSpeak.begin(client);
  
 if (WiFi.status() != WL_CONNECTED)
  {
    Serial.print("Attempting to connect to SSID: ");
    Serial.println(ssid);
    while (WiFi.status() != WL_CONNECTED)
    {
      WiFi.begin(ssid, pass);
      Serial.print(".");
      delay(5000);
    }
    Serial.println("\nConnected.");
  }

// Connect to MQTT - IBM Watson IoT Platform
  if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN)) 
  {
    Serial.println("MQTT Connected");
    mqtt.subscribe(MQTT_TOPIC_DISPLAY);

  }
  else {
    Serial.println("MQTT Failed to connect!");
    ESP.reset();
  }
 ss.begin(9600);
}

void loop()
{
  
    mqtt.loop();
  while (!mqtt.connected())
  {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (mqtt.connect(MQTT_DEVICEID, MQTT_USER, MQTT_TOKEN))
    {
      Serial.println("MQTT Connected");
      mqtt.subscribe(MQTT_TOPIC_DISPLAY);
      mqtt.loop();
    } 
    else 
    {
      Serial.println("MQTT Failed to connect!");
      delay(5000);
    }
  }
  //gps sensor
   
    if (gps.encode(ss.read()))
    {
      if (gps.location.isValid())
      {
        latitude = gps.location.lat();
        lat_str = String(latitude , 6);
        longitude = gps.location.lng();
        lng_str = String(longitude , 6);
      }
    }
     
 

  // distance 
  digitalWrite(trigger, LOW);
  delay(1);
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  T = pulseIn(echo, HIGH);
  distanceCM = T * 0.034;
  distanceCM = distanceCM / 2;
  Serial.print("Distance in cm: ");
  Serial.println(distanceCM);
  //ThingSpeak.writeField(myChannelField, ChannelField, distanceCM, myWriteAPIKey);
  delay(1000);
 
  status["distance"] = distanceCM;
  
  status["lat"]=9.94016;//CHANGE-------------------------------
  status["lng"]=76.344;
  
  serializeJson(jsonDoc, msg, 150);
  Serial.println(msg);
  
   if(distanceCM<=5)
  {
    digitalWrite(14,HIGH);
    delay(1000);
    digitalWrite(14,LOW);
    Serial.println("in the loop");
  }
   
    if (!mqtt.publish(MQTT_TOPIC, msg))
    {
      Serial.println("MQTT Publish failed");
    }
    for (int i = 0; i < 10; i++) 
    {
    mqtt.loop();
    delay(2);
    }
    
   
   
   
}
