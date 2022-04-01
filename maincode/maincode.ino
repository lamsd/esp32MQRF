#include <WiFi.h>
#include <PubSubClient.h>
#include <Wire.h>
#include <SPI.h>
#include <MFRC522.h>

#define SS_PIN 5
#define RST_PIN 27


//Variables
byte nuidPICC[4] = {0, 0, 0, 0};
MFRC522::MIFARE_Key key;
MFRC522 rfid = MFRC522(SS_PIN, RST_PIN);

const char* ssid = "TK-AI";
const char* password = "*****";
const char* mqtt_server = "*****";
const char* mqttUser = "***";
const char* mqttPassword = "***";

WiFiClient espClient;
PubSubClient client(espClient);
int value = 0;
String tess = "";

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
  SPI.begin();
 rfid.PCD_Init();
 Serial.print(F("Reader :"));
 rfid.PCD_DumpVersionToSerial();
}

void setup_wifi() {
  delay(10);
  // We start by connecting to a WiFi network
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
}

void callback(char* topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  if (String(topic) == "/topic/detected/C21282M343") {
    Serial.print("Changing output to ");
    if(messageTemp == "on"){
      Serial.println("on");
    }
    else if(messageTemp == "off"){
      Serial.println("off");
    }
  }
}

void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client", mqttUser, mqttPassword )) {
      Serial.println("connected");
      // Subscribe
      client.subscribe("/topic/detected/C21282M343");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}

void loop() {
    if (!client.connected()) {
    reconnect();
  }
  readRFID();
  client.loop(); 
}

void readRFID(void ) { /* function readRFID */
 ////Read RFID card
 for (byte i = 0; i < 6; i++) {
   key.keyByte[i] = 0xFF;
 }
 // Look for new 1 cards
 if ( ! rfid.PICC_IsNewCardPresent())
   return ;
 // Verify if the NUID has been readed
 if (  !rfid.PICC_ReadCardSerial())
   return ;
 // Store NUID into nuidPICC array
 for (byte i = 0; i < 4; i++) {
   nuidPICC[i] = rfid.uid.uidByte[i];
 }
 Serial.print(F("RFID In dec: "));
 
// client.publish("esp32/output", rfid.uid.uidByte);
 client.publish("/topic/detected/C21282M343",printDec(rfid.uid.uidByte, rfid.uid.size).c_str());
 Serial.println();
 rfid.PICC_HaltA();
}

String printDec(byte *buffer, byte bufferSize) {
 String st = "";
 String datase = "";
 for (byte i = 0; i < bufferSize; i++) {
   st +=  buffer[i];
 }
 datase += "{\"person_id\":\""+ st +"\",\"person_type\":11}";
  return datase;
}
