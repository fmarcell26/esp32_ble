
#include <WiFi.h> //119kb
#include <PubSubClient.h> //26.6kb
#include <Wire.h> //15kb
#include <BLEDevice.h> //26kb
//s#include <BLEUtils.h>
//#include <BLEScan.h>
//#include <BLEAdvertisedDevice.h>

//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////

// Replace the next variables with your SSID/Password combination
const char* ssid = "1211-Gyorsrizs";
const char* password = "rainbows";

// Add your MQTT Broker IP address :
//const char* mqtt_server = "192.168.1.144";
const char* mqtt_server = "192.168.1.144";

WiFiClient espClient;
PubSubClient client(espClient);
long lastMsg = 0;
char msg[50];
int value = 0;
bool flag = true;

int scanTime = 3; //In seconds
BLEScan* pBLEScan;
int i;
char beacon_1[20] = "EMBeacon00957";
char beacon_2[20] = "EMBeacon00957";
char beacon_3[20] = "EMBeacon00957";
int beacon1_rssi;
int beacon2_rssi;
int beacon3_rssi;

class MyAdvertisedDeviceCallbacks: public BLEAdvertisedDeviceCallbacks {
    void onResult(BLEAdvertisedDevice advertisedDevice) {
      //Serial.printf("%s \n", advertisedDevice.toString().c_str() );
          }
};
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
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

  // If a message is received on the topic esp32/output
  // turns the flag on, ans start sending the RSSI values in loop
  //if (String(topic) == "esp32/output" && String(messageTemp) == "send_data") 
  //    flag = true;
  if (String(topic) == "esp32/output" && String(messageTemp) == "stop")
      flag = false;
  
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void reconnect() {
  // Loop until we're reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    // Attempt to connect
    if (client.connect("ESP8266Client")) {
      Serial.println("connected");
      // Subscribe to desired topics
      client.subscribe("esp32/output");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
    }
  }
}

///////////////////////////////////////////////////////////////////
void setup() {
 Serial.begin(115200);
 setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);

  BLEDevice::init("");
  pBLEScan = BLEDevice::getScan(); //create new scan
  pBLEScan->setAdvertisedDeviceCallbacks(new MyAdvertisedDeviceCallbacks());//set the callback invoked, callback re hivatkozhatunk ezen a néven
  pBLEScan->setActiveScan(true); //active scan uses more power, but get results faster,
  pBLEScan->setInterval(100);//set window, how long to actively scan
  pBLEScan->setWindow(99);  // less or equal setInterval value
}
//////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////////
void loop() {
   if (!client.connected()) {
    reconnect();
  }
  //calls callback, leaves if last activity was too long time ago
  client.loop();
  
  //Start scanning and block until scanning has been completed.
  //scanTime the duration in seconds for which to scan
  //return The BLEScanResults.
  //param2 ->  are we continue scan (true) or we want to clear stored devices (false) 
  BLEScanResults foundDevices = pBLEScan->start(scanTime, false);
  BLEAdvertisedDevice MyDevice;

   for(i=0; i<foundDevices.getCount(); i++){
    MyDevice = foundDevices.getDevice(i);
      if( strcmp(MyDevice.getName().c_str(),beacon_1)==0 ){
        Serial.print(F(" RSSI : "));
        Serial.println(MyDevice.getRSSI());
        beacon1_rssi = MyDevice.getRSSI();
      }
      if( strcmp(MyDevice.getName().c_str(),beacon_2)==0 ){
        beacon2_rssi = MyDevice.getRSSI();
      }
      if( strcmp(MyDevice.getName().c_str(),beacon_3)==0 ){
        beacon3_rssi = MyDevice.getRSSI();
      }
  }

  Serial.print("Devices found: ");
  Serial.println(foundDevices.getCount());
  Serial.println("--------------------------------------------");
  pBLEScan->clearResults();   // delete results fromBLEScan buffer to release memory
  //delay(1000);
  
  long now = millis();
  //hpw often the client publishes data
  if (now - lastMsg > 3000) {
    lastMsg = now;

    // Convert the value to c string
    char str[12];
    sprintf(str, "%d", beacon1_rssi);
    char str2[12];
    sprintf(str2, "%d", beacon2_rssi);
    char str3[12];
    sprintf(str3, "%d", beacon3_rssi);
    if(flag)
      //need bluetooth information to send it here
      client.publish("esp32/beacon1_RSSI", str);
      client.publish("esp32/beacon1_RSSI", str2);
      client.publish("esp32/beacon1_RSSI", str3);
      //client.publish("esp32/beacon1_RSSI", String(MyDevice.getRSSI()));
      //client.publish("esp32/beacon1_RSSI", RSSIString);
  }  
}
