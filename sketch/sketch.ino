#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "secrets.h"

ESP8266WebServer server(80);

const int powerButtonPin = 13,
          resetButtonPin = 14,
          ledHighPin = 0,
          ledInPin = 5,
          ledOutPin = 4;

int timeToRead = 0;       

int powerPins[] = {powerButtonPin, resetButtonPin};

const int buttonNormalPressTime = 500,
          buttonForcePressTime = 5000;

int lastOn = 0,     // Last time the LED light turned on
    lastOff = 0;    // Last time the LED light turned off

int statePC = 0;  // 0=Off, 1=On, 2=Sleep
int prevLightStatus = 0;  // 0=Off, 1=On

int timeToRelease = 0;
int currentButtonPressed = 0;  // 0=Nothing, 1=Power On/Off/Force, 2=Reboot

void pressPowerButton(int time){
  digitalWrite(powerPins[currentButtonPressed-1], HIGH);
  timeToRelease = millis() + time;
}

void pressResetButton(int time){
  digitalWrite(powerPins[currentButtonPressed-1], HIGH);
  timeToRelease = millis() + time;
}

void checkIfNeedToRelease(){
  if (currentButtonPressed && millis() >= timeToRelease){
    digitalWrite(powerPins[currentButtonPressed - 1], LOW);
    currentButtonPressed = 0;
  }
}

void checkLEDPowerLight(){
  int stateLED = digitalRead(ledInPin);
  if (millis() >= timeToRead){
    Serial.println(stateLED);
    timeToRead += 500;
  }
  digitalWrite (ledOutPin, stateLED);
  if (stateLED && !prevLightStatus){
    lastOn = millis();
  } else if (!stateLED && prevLightStatus) {
    lastOff = millis();
  }
  prevLightStatus = stateLED;
  checkStatusChange(stateLED);
}

void checkStatusChange(int stateLED){
  if(lastOn != 0 && lastOff != 0){
    if((millis() < lastOn + 4000) && (millis() < lastOff + 4000)){
      statePC = 2;
      return;
    }
  }
  if(stateLED){
    statePC = 1;
  } else {
    statePC = 0;
  }
}

void handleRoot(){
  String rootPage;
  rootPage += "<html style=\"font-family:Arial,Helvetica,sans-serif\"><head><title>Manage PC</title></head><body style=\"height:100vh;width:100vw;display:grid;place-items:center;padding:0;margin:0\"><a href=\"onoff\" style=\"font-size:3em;color:#000;text-decoration:none;height:100%;width:100%;border-bottom:2px solid #000;display:grid;place-items:center\" onmouseover='this.style.backgroundColor=\"#000000\",this.style.color=\"white\"' onmouseout='this.style.backgroundColor=\"\",this.style.color=\"black\"'>ON/OFF</a><a href=\"reboot\" style=\"font-size:3em;color:#000;text-decoration:none;height:100%;width:100%;border-bottom:2px solid #000;display:grid;place-items:center\" onmouseover='this.style.backgroundColor=\"#000000\",this.style.color=\"white\"' onmouseout='this.style.backgroundColor=\"\",this.style.color=\"black\"'>RESTART</a><a href=\"force\" style=\"font-size:3em;color:#000;text-decoration:none;height:100%;width:100%;border-bottom:2px solid #000;display:grid;place-items:center\" onmouseover='this.style.backgroundColor=\"#000000\",this.style.color=\"white\"' onmouseout='this.style.backgroundColor=\"\",this.style.color=\"black\"'>FORCE</a></body></html>";
  server.send(200, "text/html", rootPage);
}

void handleOnOff(){
  server.sendHeader("Location", String("/"), true);
  if (!currentButtonPressed){
    currentButtonPressed = 1;
    pressPowerButton(buttonNormalPressTime);
  }
  server.send ( 302, "text/plain", "");
}

void handleReboot(){
  server.sendHeader("Location", String("/"), true);
  if (!currentButtonPressed){
    currentButtonPressed = 2;
    pressResetButton(buttonNormalPressTime);
  }
  server.send ( 302, "text/plain", "");
}

void handleForce(){
  server.sendHeader("Location", String("/"), true);
  if (!currentButtonPressed){
    currentButtonPressed = 1;
    pressPowerButton(buttonForcePressTime);
  }
  server.send ( 302, "text/plain", "");
}

void setup(){
  Serial.begin(115200);
  Serial.println("");

  for (int i=0; i<sizeof(powerPins); i++){
    pinMode(powerPins[i], OUTPUT);
    digitalWrite(powerPins[i], LOW);
  }
  
  pinMode(ledOutPin, OUTPUT);
  digitalWrite(ledOutPin, LOW);
  pinMode(ledInPin, INPUT);
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(SECRET_SSID, SECRET_PWD);
  Serial.print("Connecting");
  
  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", handleRoot);
  server.on("/onoff", handleOnOff);
  server.on("/reboot", handleReboot);
  server.on("/force", handleForce);
  // server.onNotFound(handleNotFound);
  server.begin();

  pinMode(ledHighPin, OUTPUT);
  digitalWrite(ledHighPin, HIGH);
}

void loop(){
  checkIfNeedToRelease();
  checkLEDPowerLight();
  server.handleClient();
}
