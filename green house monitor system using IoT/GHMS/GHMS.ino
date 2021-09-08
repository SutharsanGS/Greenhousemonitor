/*
  Modified by Ardhi Wahyudhi
  Using  ArduinoJson 5.13.5 dan CTBot 1.4.1 library 
*/
#include <ESP8266WiFi.h>
#include "CTBot.h"
#include <PubSubClient.h>
CTBot myBot;
#include "DHT.h"
#define DHTPIN D4    // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11   // DHT 11
DHT dht(DHTPIN, DHTTYPE);
#define WIFI_SSID "Mia3"
#define WIFI_PASSWORD "Sudharsan"

// Raspberri Pi Mosquitto MQTT Broker
#define MQTT_HOST IPAddress(13, 127, 144, 226)
// For a cloud MQTT broker, type the domain name
//#define MQTT_HOST "example.com"
#define MQTT_PORT 1883

// Temperature MQTT Topics
#define MQTT_PUB_TEMP "iot/temp"
#define MQTT_PUB_HUM "iot/hum"
String ssid  = "Mia3"; 
String pass  = "Sudharsan"; 
String token = "1813048697:AAHZt3qX0ubEexW57e7OLDu5fbj3-a0N560";                        
const int sensor_pin = A0;

const char* mqtt_server = "13.127.144.226";
WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;

void setup_wifi() {

  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  randomSeed(micros());

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}
void callback(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();

  // Switch on the LED if an 1 was received as first character
  if ((char)payload[0] == '1') {
    digitalWrite(BUILTIN_LED, LOW);   // Turn the LED on (Note that LOW is the voltage level
    // but actually the LED is on; this is because
    // it is acive low on the ESP-01)
  } else {
    digitalWrite(BUILTIN_LED, HIGH);  // Turn the LED off by making the voltage HIGH
  }

}
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("outTopic", "hello world");
      // ... and resubscribe
      client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}


void setup() {
  // initialize the Serial
  Serial.begin(115200);
  Serial.println("Starting TelegramBot...");
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  // connect the ESP8266 to the desired access point
  myBot.wifiConnect(ssid, pass);
  delay(10000);

  // set the telegram bot token
  myBot.setTelegramToken(token);

  // check if all things are ok
  if (myBot.testConnection())
    Serial.println("\ntestConnection OK");
  else
    Serial.println("\ntestConnection NOT OK");

   Serial.begin(115200);
  Serial.println();

  dht.begin();
  
  

 
}

void loop() {
     
  // a variable to store telegram message data
  TBMessage msg;
   float moisture_percentage;

  moisture_percentage = ( 100.00 - ( (analogRead(sensor_pin)/1023.00) * 100.00 ) );
   
   float h = dht.readHumidity();
  // Read temperature as Celsius (the default)
  float t = dht.readTemperature();
  // Read temperature as Fahrenheit (isFahrenheit = true)
  float f = dht.readTemperature(true);
  String mois = "Soil Humidity:" + String(moisture_percentage) + " % \n";
  String moismq=String(moisture_percentage);
  String humid = "Humidity: " + String(h) + " % \n";
  String humidmq= String(h);
  String temp = "Temperature: " + String(t) + "°C, " + String(f) + "°F \n";
  String tempmq=String(t);
  
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  long now = millis();
  if (now - lastMsg > 2000) {
    lastMsg = now;
    ++value;
    
    Serial.print("Publish message: ");
    (char*)humid.c_str();
    client.publish("iot/hum", (char*) humidmq.c_str());
    client.publish("iot/temp", (char*) tempmq.c_str());
    client.publish("iot/soil", (char*) moismq.c_str());
  } 

  // if there is an incoming message...
  if (myBot.getNewMessage(msg)) {
    if (msg.text.equalsIgnoreCase("Humidity")) {    
     myBot.sendMessage(msg.sender.id, humid);
     delay(5000);
    }
 if (msg.text.equalsIgnoreCase("Temperature")) {    
      myBot.sendMessage(msg.sender.id, temp); 
      delay(5000);
      }
    if (msg.text.equalsIgnoreCase("Soil moisture")) {    
      myBot.sendMessage(msg.sender.id,mois); 
      delay(5000);
    }}
    else {                                                    
      // generate the message for the sender
      String reply;
      reply = (String)"Welcome " + msg.sender.username + (String)". Try to send 'Hello' or 'Thank you'.";
                      
    }
   
  
  
  delay(500); // wait 500 milliseconds
}
