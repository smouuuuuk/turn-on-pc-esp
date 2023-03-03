#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "secrets.h"

ESP8266WebServer server(80);

void handleRoot(){
  String rootPage;
  rootPage += "<html style=\"font-family:Arial,Helvetica,sans-serif\"><head><title>Manage PC</title></head><body style=\"height:100vh;width:100vw;display:grid;place-items:center;padding:0;margin:0\"><a href=\"onoff\" style=\"font-size:3em;color:#000;text-decoration:none;height:100%;width:100%;border-bottom:2px solid #000;display:grid;place-items:center\" onmouseover='this.style.backgroundColor=\"#000000\",this.style.color=\"white\"' onmouseout='this.style.backgroundColor=\"\",this.style.color=\"black\"'>ON/OFF</a><a href=\"reboot\" style=\"font-size:3em;color:#000;text-decoration:none;height:100%;width:100%;border-bottom:2px solid #000;display:grid;place-items:center\" onmouseover='this.style.backgroundColor=\"#000000\",this.style.color=\"white\"' onmouseout='this.style.backgroundColor=\"\",this.style.color=\"black\"'>RESTART</a><a href=\"force\" style=\"font-size:3em;color:#000;text-decoration:none;height:100%;width:100%;border-bottom:2px solid #000;display:grid;place-items:center\" onmouseover='this.style.backgroundColor=\"#000000\",this.style.color=\"white\"' onmouseout='this.style.backgroundColor=\"\",this.style.color=\"black\"'>FORCE</a></body></html>";
  server.send(200, "text/html", rootPage);
  
}

void setup(){
  Serial.begin(115200);
  Serial.println("");
  
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
//  server.on("/onoff", handleOnOff);
//  server.on("/reboot", handleRoot);
//  server.on("/force", handleRoot);
  // server.onNotFound(handleNotFound);
  server.begin();
}

void loop(){
  server.handleClient();
}
