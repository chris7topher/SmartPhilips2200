// Load libraries
#include <ESP8266WiFi.h>
#include <SoftwareSerial.h>
#include <PubSubClient.h>

#if defined(ESP8266) && !defined(D5)
#define D5 (14)
#define D6 (12)
#define D7 (13)
#endif

//SoftwareSerial to read Display TX (D5 = RX, D6 = unused)
SoftwareSerial swSer (D5, D6);

//serial Input
char serInCommand[39];
char serInCommand_old[39];
unsigned long timestampLastSerialMsg;
byte serInIdx = 0;

//commands
byte powerOn[] =      {0xd5, 0x55, 0x01, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x35, 0x05};
byte requestInfo[] =  {0xD5, 0x55, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x00, 0x01, 0x14};
byte powerOff[] =     {0xd5, 0x55, 0x00, 0x01, 0x00, 0x00, 0x02, 0x01, 0x00, 0x00, 0x0d, 0x19};
byte hotWater[] =     {0xD5, 0x55, 0x00, 0x01, 0x00, 0x00, 0x02, 0x04, 0x00, 0x00, 0x31, 0x23};
byte espresso[] =     {0xD5, 0x55, 0x00, 0x01, 0x00, 0x00, 0x02, 0x02, 0x00, 0x00, 0x19, 0x0F};
byte coffee[] =       {0xD5, 0x55, 0x00, 0x01, 0x00, 0x00, 0x02, 0x08, 0x00, 0x00, 0x29, 0x3E};
byte steam[] =        {0xD5, 0x55, 0x00, 0x01, 0x00, 0x00, 0x02, 0x10, 0x00, 0x00, 0x19, 0x04};
byte coffeePulver[] = {0xD5, 0x55, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x02, 0x00, 0x19, 0x0D};
byte coffeeWater[] =  {0xD5, 0x55, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x04, 0x00, 0x30, 0x27}; 
byte calcNclean[]=    {0xD5, 0x55, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x20, 0x00, 0x38, 0x15};
byte aquaClean[] =    {0xD5, 0x55, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x10, 0x00, 0x1D, 0x14};
byte startPause[] =   {0xD5, 0x55, 0x00, 0x01, 0x00, 0x00, 0x02, 0x00, 0x00, 0x01, 0x09, 0x10};

char oldStatus[19] = "";


// Replace with your network credentials
const char* ssid     = "yourWifiSSID";
const char* password = "yourWifiPassword";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "192.168.xxx.xxx";
const int mqtt_port = 1883;
// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient espClient;
PubSubClient client(espClient);


// Don't change the function below. 
// This function connects your ESP8266 to your router
void setup_wifi() {
  delay(500);
  // We start by connecting to a WiFi network
  Serial1.println();
  Serial1.print("Connecting to ");
  Serial1.println(ssid);
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial1.print(".");
  }
  Serial1.println("");
  Serial1.print("WiFi connected - ESP IP address: ");
  Serial1.println(WiFi.localIP());
}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something
void callback(String topic, byte* message, unsigned int length) {
  
  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  messageTemp += '\n';
  
  
  if(topic=="coffee/command/custom"){
    //For custom hex commands
      runCustomCommand(messageTemp, length);
  } else{
    //For predefined commands
    int count = messageTemp.toInt();
    if(count < 0 || count > 99){
      Serial1.println("Count out of range");
    } else if(topic=="coffee/command/powerOn"){
        serialSend(powerOn, count);
        //Workaround: D7 is connected to a NPN Transistor that cuts the ground from the display
        //--> MC of the display reboots
        digitalWrite(D7, LOW);
        delay(500);
        digitalWrite(D7, HIGH);
      } else if(topic=="coffee/command/powerOff"){
          serialSend(powerOff, count);    
      } else if(topic=="coffee/command/hotWater"){
          serialSend(hotWater, count);    
      } else if(topic=="coffee/command/espresso"){
          serialSend(espresso, count);    
      } else if(topic=="coffee/command/coffee"){
          serialSend(coffee, count);    
      } else if(topic=="coffee/command/steam"){
          serialSend(steam, count);    
      } else if(topic=="coffee/command/coffeePulver"){
          serialSend(coffeePulver, count);    
      } else if(topic=="coffee/command/coffeeWater"){
          serialSend(coffeeWater, count);    
      } else if(topic=="coffee/command/calcNclean"){
          serialSend(calcNclean, count);    
      } else if(topic=="coffee/command/aquaClean"){
          serialSend(aquaClean, count);    
      } else if(topic=="coffee/command/startPause"){
          serialSend(startPause, count);    
      } 
    }
}

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266 
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial1.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "CoffeeMachine";
    // Attempt to connect
    if (client.connect(clientId.c_str(), "admin", "admin")) {
      Serial1.println("connected");  
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more outputs)
      client.subscribe("coffee/command/restart");
      client.subscribe("coffee/command/powerOn");
      client.subscribe("coffee/command/powerOff");  
      client.subscribe("coffee/command/requestInfo");
      client.subscribe("coffee/command/hotWater");
      client.subscribe("coffee/command/espresso");
      client.subscribe("coffee/command/coffee");
      client.subscribe("coffee/command/steam");
      client.subscribe("coffee/command/coffeePulver");
      client.subscribe("coffee/command/coffeeWater");
      client.subscribe("coffee/command/calcNclean");
      client.subscribe("coffee/command/aquaClean");
      client.subscribe("coffee/command/startPause");
      client.subscribe("coffee/command/custom");
      
    } else {
      Serial1.print("failed, rc=");
      Serial1.print(client.state());
      Serial1.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

void setup() {
  // Serial1 port for debugging purposes
  Serial1.begin(115200);
  // Serial communication with coffee machine (Display + ESP)
  Serial.begin(115200);
  // Serial communication to read display TX and give it over to Serial
  swSer.begin(115200);
  
  setup_wifi();
  client.setServer(mqtt_server, mqtt_port);
  client.setCallback(callback);
  client.publish("coffee/status", "ESP_STARTUP");

  pinMode(D7, OUTPUT);
  digitalWrite(D7, HIGH);
}

void serialReadPublish(char newStatus[]){
  if(strcmp(newStatus, oldStatus) != 0){
    client.publish("coffee/status", newStatus);
    strncpy(oldStatus, newStatus, 19);
  }
}

void runCustomCommand(String customCmd, unsigned int length) {
  byte data2send[length];
  for (int i = 0; i < length/2; i++) {
    byte extract;
    char a = (char)customCmd[2*i];
    char b = (char)customCmd[2*i + 1];
    extract = convertCharToHex(a)<<4 | convertCharToHex(b);
    data2send[i] = extract;
  }
  Serial.write(data2send, length/2);
}

void serialSend(byte command[], int sendCount) {
  for (int i = 0; i <= sendCount; i++) {
    Serial.write(command, 12);
  }
}

void loop() {
  if (!client.connected()) {
    reconnect();
  }
  client.loop();
  
  while (swSer.available() > 0) {
    Serial.write(swSer.read());
  }
  
  while(Serial.available() > 0) {
    char b = Serial.read();
    sprintf(&serInCommand[serInIdx],"%02x", b);
    serInIdx += 2; 

    //Skip input if it doesn't start with 0xd5
    if(serInIdx == 2 && b != 0xd5) {
      serInIdx = 0;
    }
    if(serInIdx > 37){
      serInCommand[38] = '\0';
	  timestampLastSerialMsg = millis();
      if(strcmp(serInCommand, serInCommand_old) != 0){
        client.publish("coffee/status", serInCommand, 38);
        memcpy( serInCommand_old, serInCommand, 39);
      }
      serInIdx = 0;
    }
  }
  //Send signal that the coffee machine is off if there is no incomming message for more than 3 seconds
  if(timestampLastSerialMsg != 0 && millis() - timestampLastSerialMsg > 3000){
    client.publish("coffee/status", "d5550100000000000000000000000000000626", 38);
    timestampLastSerialMsg = 0;
  }
}

char convertCharToHex(char ch)
{
  char returnType;
  switch(ch)
  {
    case '0':
    returnType = 0;
    break;
    case  '1' :
    returnType = 1;
    break;
    case  '2':
    returnType = 2;
    break;
    case  '3':
    returnType = 3;
    break;
    case  '4' :
    returnType = 4;
    break;
    case  '5':
    returnType = 5;
    break;
    case  '6':
    returnType = 6;
    break;
    case  '7':
    returnType = 7;
    break;
    case  '8':
    returnType = 8;
    break;
    case  '9':
    returnType = 9;
    break;
    case  'A':
    returnType = 10;
    break;
    case  'B':
    returnType = 11;
    break;
    case  'C':
    returnType = 12;
    break;
    case  'D':
    returnType = 13;
    break;
    case  'E':
    returnType = 14;
    break;
    case  'F' :
    returnType = 15;
    break;
    case  'a':
    returnType = 10;
    break;
    case  'b':
    returnType = 11;
    break;
    case  'c':
    returnType = 12;
    break;
    case  'd':
    returnType = 13;
    break;
    case  'e':
    returnType = 14;
    break;
    case  'f' :
    returnType = 15;
    break;
    default:
    returnType = 0;
    break;
  }
  return returnType;
}
