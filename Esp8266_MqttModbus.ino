#define ARDUINOJSON_USE_LONG_LONG 1

#include <ArduinoJson.h>
#include <ModbusMaster232.h>
#include<ESP8266WiFi.h>
#include<PubSubClient.h>
#include <stdlib.h>

#define TX_ENABLE_PIN       D4
#define RE                  D3
#define READ_REGISTER       10
#define READ_START_ADD      40000

const char* mqtt_Server = "XXX.XX.XXX.X";
const char* ssid = "XXXXXXXX";
const char* password = "XXXXXXX";

ModbusMaster232 node(1, TX_ENABLE_PIN);
WiFiClient espClient;
PubSubClient client(espClient);

void modbusRead(void);
void setup_wifi(void);
void reconnect(void);

void setup() 
{
  Serial.begin(9600);
  node.begin(9600);
  setup_wifi();
  client.setServer( mqtt_Server , 1883 );
  pinMode(RE, OUTPUT);
}

void loop() 
{
  modbusRead();
  delay(1000);
  
}

void modbusRead(void) 
{
  uint16_t response[12] = { 0 };
  DynamicJsonBuffer jBuffer(600);
  char jsonMessageBuffer[600];
  JsonObject &jsonEncoder = jBuffer.createObject();

  float floatValue;
  int modbusReadingStatus = 0;
  int readAddBuff = READ_START_ADD;
  modbusReadingStatus = node.readHoldingRegisters(READ_START_ADD, READ_REGISTER);
  digitalWrite(RE, HIGH);

  if (modbusReadingStatus == 0) 
  {
    Serial.println("Modbus Read OK..");
    if (!client.connected()) 
    {
      reconnect();
    }
    else 
    {
      Serial.println("MQTT server Connected");
      for (uint8_t i = 0; i < READ_REGISTER ; i++)
      {
        Serial.print(readAddBuff);
        Serial.print(" =\t");
        response[i] = node.getResponseBuffer(i);
        Serial.println(response[i]);
        jsonEncoder[String(readAddBuff)] = String(response[i]);
        jsonEncoder.printTo(jsonMessageBuffer, sizeof(jsonMessageBuffer));
        readAddBuff += 1;
      }
      client.publish("MODBUS/test", jsonMessageBuffer);
    }
    node.clearResponseBuffer();
  }
  else 
  {
    Serial.println("Modbus Read FAIL..");
  }

}

void setup_wifi() 
{
  delay(10);
  Serial.print("Connecting to ");
  Serial.println(ssid);
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) 
  {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP8266Client-";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str(),"XXXXX","XXXXXX")) {
      Serial.println("connected");
      // Once connected, publish an announcement...
      client.publish("MODBUS/test", "Modbus client connected");
      // ... and resubscribe
      // client.subscribe("inTopic");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}
