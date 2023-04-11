/* 
 * Turning on your PC using an ESP8266 Microcontroller
 * Created March 6, 2023 by Alexander Cabello
 */

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>

#include "secrets.h"            // files with credentials, the following variables are taken from the secrets.h file:
                                // SECRET_SSID  - SSID from the AP used to connect to the Internet
                                // SECRET_PWD   - Password to access the network
                                // SECRET_REALM - 
                                // SECRET_LOGIN  - User to authenticate in the WebServer
                                // SECRET_PASS  - Password to authenticate in the WebServer

ESP8266WebServer server(80);    // WebServer will be hosted on port 80

WiFiClient wifiClient;

const int powerButtonPin = 13,  // pins needed for the project
          resetButtonPin = 14,
          ledInPin = 5,
          ledOutPin = 4;

int powerPins[] = {powerButtonPin, resetButtonPin};   // pins that will act as the power and restart "buttons"

const int buttonNormalPressTime = 500,  // press the button for 500ms
          buttonForcePressTime = 5000;  // press the button for 5 seconds

int lastOn = 0,     // last time the LED light turned on (since the start)
    lastOff = 0;    // last time the LED light turned off (since the start)

int statePC = 0;    // 0=Off, 1=On, 2=Sleep

const String stateColors[] = {"#f03131", "#5cdf28", "#3743f0"};  // rgb colors in hexadecimal format that will represent the different states of the PC

int prevLightStatus = 0;       // 0=Off, 1=On, not the immediate value of the LED, will be useful to determine if there's been a change from its previous state

int timeToRelease = 0;         // time at which the currently pressed button will have to be released
int currentButtonPressed = 0;  // 0=Nothing, 1=Power On/Off/Force, 2=Reboot

String currentPublicIP = "";
const int waitTimeCheckIP = 4 * 3600 * 1000;  // will send and API request every 4 hours
int timeToCheckPublicIP = 0;   // time at which the API request will be sent

/**
 * "Presses" the button by putting the pin assigned to the power button to its HIGH state
 * Takes in one parameter - time - which defines for how long the power button will be pressed
 */
void pressPowerButton(int time){
  digitalWrite(powerPins[currentButtonPressed-1], HIGH);
  // the time at which the button will have to be released is stored
  timeToRelease = millis() + time;
}

/**
 * Does the same as the previous function but for the reset button by putting the pin assigned to the reset button to its HIGH state
 */
void pressResetButton(int time){
  digitalWrite(powerPins[currentButtonPressed-1], HIGH);
  timeToRelease = millis() + time;
}

/**
 * Checks if there is a pressed button and if the current time has reached the time at which the button has to be released
 * If so, the pin assigned to the pressed button is pulled to its LOW state
 */
void checkIfNeedToRelease(){
  if (currentButtonPressed && millis() >= timeToRelease){
    digitalWrite(powerPins[currentButtonPressed - 1], LOW);
    currentButtonPressed = 0;
  }
}

/**
 * Checks the present state of the Power LED Light coming from the Motherboard
 * Also stores the last time at which the LED turned ON or OFF (implying a transition from one to the other)
 */
void checkLEDPowerLight(){
  int stateLED = digitalRead(ledInPin);
  // outputs its current state to the output pin going to the LED of the button on the case
  digitalWrite (ledOutPin, stateLED);
  // if it's ON and it was OFF in its previous state
  if (stateLED && !prevLightStatus){
    lastOn = millis();
  // if it's OFF and it was ON in its previous state
  } else if (!stateLED && prevLightStatus) {
    lastOff = millis();
  }
  // current state stored for comparison in the next loop
  prevLightStatus = stateLED;
  checkStatusChange(stateLED);
}

/**
 * Checks the current state of the PC
 * The state can be one of the following: 0 (OFF), 1 (ON), 2 (SLEEP)
 * Takes one parameter - stateLED - which defines the current state of the LED
 */
void checkStatusChange(int stateLED){
  // necessary for when the code starts to run, otherwise it would define the state of the PC as SLEEP at the start
  if(lastOn != 0 && lastOff != 0){
    // if the LED has neither been ON or OFF for more than 4 seconds it must mean it's in SLEEP which goes from ON to OFF every second
    if((millis() < lastOn + 4000) && (millis() < lastOff + 4000)){
      // if the previous state is different then we announce the change it on the Serial Monitor
      if (statePC != 2) Serial.println("State: Sleep");
      statePC = 2;
      return;
    }
  }
  // if it's not in SLEEP it must be either ON or OFF
  // we can just use the current state on the LED to determine that
  if(stateLED){
    if (statePC != 1) Serial.println("State: On");
    statePC = 1;
  } else {
    if (statePC != 0) Serial.println("State: Off");
    statePC = 0;
  }
}

/**
 * Serves the root page to the client accessing the WebServer
 * Will vary the color of a small portion in the background indicating the current state of the PC
 */
void handleRoot(){
  // we include all of the html code on a String which will be sent to the client
  String rootPage;
  rootPage += "<html style=\"font-family:Arial,Helvetica,sans-serif; box-sizing: border-box; display: grid; place-items: center; height: 100vh; padding: 1em; background-color: ";
  rootPage += stateColors[statePC];
  rootPage += "\"> <head> <title>Manage PC</title> </head> <body style=\"height:100%;width:100%;display:grid;place-items:center;padding:0;margin:0; border: 1px solid black; background-color: white;\"> <a href=\"onoff\" style=\"font-size:3em;color:#000;text-decoration:none;height:100%;width:100%;border-bottom:2px solid #000;display:grid;place-items:center\" onmouseover='this.style.backgroundColor=\"#000000\",this.style.color=\"white\"' onmouseout='this.style.backgroundColor=\"\",this.style.color=\"black\"'>ON/OFF</a> <a href=\"reboot\" style=\"font-size:3em;color:#000;text-decoration:none;height:100%;width:100%;border-bottom:2px solid #000;display:grid;place-items:center\" onmouseover='this.style.backgroundColor=\"#000000\",this.style.color=\"white\"' onmouseout='this.style.backgroundColor=\"\",this.style.color=\"black\"'>RESTART</a> <a href=\"force\" style=\"font-size:3em;color:#000;text-decoration:none;height:100%;width:100%;border-bottom:2px solid #000;display:grid;place-items:center\" onmouseover='this.style.backgroundColor=\"#000000\",this.style.color=\"white\"' onmouseout='this.style.backgroundColor=\"\",this.style.color=\"black\"'>FORCE</a> </body> </html>";
  server.send(200, "text/html", rootPage);
}

/**
 * Serves the onoff page to the client accessing the WebServer
 * If no button is currently pressed it will call the function to press the Power Button to turn the PC ON or OFF
 * At the end, it redirects the client to the root page
 */
void handleOnOff(){
  server.sendHeader("Location", String("/"), true);
  if (!currentButtonPressed){
    currentButtonPressed = 1;
    pressPowerButton(buttonNormalPressTime);
  }
  server.send ( 302, "text/plain", "");
}

/**
 * Serves the reboot page to the client accessing the WebServer
 * If no button is currently pressed it will call the function to press the Reset Button
 * At the end, it redirects the client to the root page
 */
void handleReboot(){
  server.sendHeader("Location", String("/"), true);
  if (!currentButtonPressed){
    currentButtonPressed = 2;
    pressResetButton(buttonNormalPressTime);
  }
  server.send ( 302, "text/plain", "");
}

/**
 * Serves the force page to the client accessing the WebServer
 * If no button is currently pressed it will call the function to press the Power Button and hold it for the specified time
 * At the end, it redirects the client to the root page
 */
void handleForce(){
  server.sendHeader("Location", String("/"), true);
  if (!currentButtonPressed){
    currentButtonPressed = 1;
    pressPowerButton(buttonForcePressTime);
  }
  server.send ( 302, "text/plain", "");
}

/**
 * If the client tries to access a location that doesn't exist they will be redirected to the root page
 */
void handleNotFound(){
  server.sendHeader("Location", String("/"), true);
  server.send ( 302, "text/plain", "");
}

void checkIfNeedToCheckIP() {
  if (millis() >= timeToCheckPublicIP){
    HTTPClient http;
    http.begin(wifiClient, "https://api.ipify.org");
    int httpCode = http.GET();
    if (httpCode > 0) {
      String payload = http.getString();
      Serial.println(payload);
      if (payload != currentPublicIP){
        currentPublicIP = payload;
    }
    http.end();
    }
    timeToCheckPublicIP += waitTimeCheckIP;
  }
}

void setup(){
  Serial.begin(115200);
  Serial.println("");

  // pin initialization
  for (int i=0; i<sizeof(powerPins); i++){
    pinMode(powerPins[i], OUTPUT);
    digitalWrite(powerPins[i], LOW);
  }
  
  pinMode(ledOutPin, OUTPUT);
  digitalWrite(ledOutPin, LOW);
  pinMode(ledInPin, INPUT);

  // connection to the AP
  WiFi.mode(WIFI_STA);
  WiFi.begin(SECRET_SSID, SECRET_PWD);
  Serial.print("Connecting");
  
  // wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(WiFi.SSID());
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  // functions that determine what happens when the client asks for a certain page, in this case we have a root page and three more pages for the different actions (turning on/off the PC, restarting it or forcing it to shut down)
  // when accessing the root page or any other directory we'll be prompted with a pop-up to authenticate ourselves (if we haven't authenticated ourselves previously or some time has passed)
  server.on("/", []() {
    // if the user/password combination don't match the defined credentials in the secrets.h file it will display a fail response
    // if it matches it continues with the code
    if (!server.authenticate(SECRET_LOGIN, SECRET_PASS)){
      return server.requestAuthentication(DIGEST_AUTH, SECRET_REALM, "Authentication Failed");
    }
    handleRoot();
  });
  server.on("/onoff", handleOnOff);
  server.on("/reboot", handleReboot);
  server.on("/force", handleForce);
  server.onNotFound(handleNotFound);
  server.begin();
}

void loop(){
  checkIfNeedToRelease();
  checkLEDPowerLight();
  checkIfNeedToCheckIP();
  server.handleClient();
}
