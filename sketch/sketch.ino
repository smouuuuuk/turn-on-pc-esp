#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>

#include "secrets.h"

ESP8266WebServer server(80);

void handleRoot(){
  String rootPage;
  rootPage += "<html> <head> <title>Manage PC</title> </head> <body> <h1>HOLA!!</h1> </body> </html>";
  server.send(200, "text/html", rootPage);
  
}

void setup(){
  Serial.begin(115200);
  Serial.println("");
  
  WiFi.mode(WIFI_STA);
  WiFi.begin(SECRET_SSID, SECRET_PWD);
  
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
  // server.onNotFound(handleNotFound);
  server.begin();
}

void loop(){
  server.handleClient();
}
