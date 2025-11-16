//   Demo code for ESP32

#include <WiFi.h>
#include <WebServer.h>

#include "wpage.h"  //Web page in separate file

//Attributes of a WiFi access point: name, password and IP-address
const char* ssid = "ESP32-Start";
const char* password = "12345678";
IPAddress local_ip(192,168,1,1);
IPAddress gateway(192,168,1,1);
IPAddress subnet(255,255,255,0);

WebServer server(80); //WiFi server object

//Onboard led and button
#define SYSTEM_LED 2
#define BUTTON 0

//Variables for web page interaction
uint16_t from_webpage;
uint16_t to_webpage;

unsigned long timestamp;

//---------------------------------------------------------------------------------

//  WiFi server routines

void handleRoot() {
  String s = MAIN_page; //Read HTML contents from web page file
  server.send(200, "text/html", s); //Send web page to client
}

void handleLED() {
  String ledState = "unknown"; //Feedback parameter
  String par = server.arg("LEDstate"); //Refer to xhttp.open("GET", "setLED?LEDstate="+led, true);
  if (par == "0") {
    digitalWrite(SYSTEM_LED,LOW);
    ledState = "Onboard LED off";
  }
  if (par == "1")
  {
    digitalWrite(SYSTEM_LED,HIGH);
    ledState = "Onboard LED on";
  }
  server.send(200, "text/plane", ledState); //Send feedback to led label
}

void handleCMD() {
  String par = server.arg("valStr"); //Refer to  xhttp.open("GET", "setVal?valStr="+str, true);
  if (par!="") from_webpage = par.toInt();
}

void handleDataRequest(){
  String s = "data="+String(to_webpage)+"|"+digitalRead(BUTTON); //Variable and button state
  server.send(200, "text/plane", s);
}

//---------------------------------------------------------------------------------

void handleSerial() { //Serial port routine
  delay(3); //wait for all characters
  char buf[128];
  Serial.readBytes(buf,Serial.available());
  String s = buf; //char* -> String
  to_webpage = s.toInt();
}

//---------------------------------------------------------------------------------

void setup(){
  digitalWrite(SYSTEM_LED, HIGH);
  pinMode(SYSTEM_LED,OUTPUT);
  pinMode(BUTTON,INPUT);
  delay(200);      
  Serial.begin(115200);

   //Configure WiFi access point
  WiFi.mode(WIFI_AP);
  delay(10);
  WiFi.softAPConfig(local_ip, gateway, subnet);
  delay(10);
  WiFi.softAP(ssid, password);
  delay(100);

  digitalWrite(SYSTEM_LED, LOW);

   //Assign routines for client requests
  server.on("/", handleRoot); // Main page
  server.on("/setLED", handleLED);
  server.on("/setVal", handleCMD);
  server.on("/data", handleDataRequest);
 
  server.begin();

  from_webpage = 1111;
  to_webpage = 0;

  timestamp = millis();

  digitalWrite(SYSTEM_LED, HIGH);
}

//----------------------------------------------------------------------------------------

void loop(){
  server.handleClient();          //Handle client requests

  if (Serial.available()) handleSerial();

  unsigned long current_time = millis();
  if ((current_time - timestamp) > 50) {
    timestamp = current_time;
    //This part run once in 50 ms
    Serial.println(String(from_webpage));
    to_webpage++;
  }
}
