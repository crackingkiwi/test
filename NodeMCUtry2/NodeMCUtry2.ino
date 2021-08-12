//What libraries are implicitly included??
//how to replace pinMode by DDR instruction?

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

const char *ssid = "LB2.4G202PDCN";
const char *password = "zhao63148842";

#define LED 2
 
ESP8266WebServer server(80);

void handleRoot();
void handleNotFound();
void handleLED();

void setup() {

  int i=0;
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  pinMode(LED,1);
  
  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");
  while(WiFi.status()!=WL_CONNECTED) {
    delay(1000);
    i++;
    Serial.print(i);
    }
  Serial.println("\nConnected!");
  Serial.print("IP: "); Serial.println(WiFi.localIP());

  if(MDNS.begin("nodemcu"))//notworking, don't know why...
  {
    Serial.println("mDNS reponder started");
  }
  
  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);

  server.on("/LED", HTTP_POST, handleLED);
  server.on("/login", HTTP_POST, handlelogin);

  server.begin();
  Serial.println("HTTP server started");
}

void loop()
{
  server.handleClient();
}

void handleRoot()
{
  //server.send(200, "text/html", "<form action=\"/LED\" method=\"POST\">"
  //"<input type=\"submit\" value=\"Toggle LED\"></form>");
  server.send(200, "text/html", 
  "<form action=\"/login\" method=\"POST\">" 
  "<input type=\"text\" name=\"username\" placeholder=\"Username\"></br>"  
  "<input type=\"password\" name=\"password\" placeholder=\"Password\"></br>" 
  "<input type=\"submit\" value=\"Login\">"   
  "</form>" 
  "<p>Try 'JD' and 'ps123' ... </p>"
  "<form action=\"/LED\" method=\"POST\">"
  "<input type=\"submit\" value=\"Toggle the led\">"
  "</form>"
  );
}

void handleNotFound()
{
  server.send(404, "text/plain", "404: Not Foud");
}

void handlelogin()
{
  if(!server.hasArg("username") || !server.hasArg("password")  || 
  server.arg("username")==NULL || server.arg("password")==NULL)
  {
    server.send(400, "text/plain", "400: Invalid Request");
    return;
  }
  if(server.arg("username")=="JD" && server.arg("password")=="ps123")
  {
    server.send(200, "text/html", 
    "<h1>Welcome, " + server.arg("username") + "</h1> <p>Login successful</p>");
  }
  else
  {
    server.send(401, "text/plain", "401: Unauthorized");
  }
}
void handleLED()
{
  digitalWrite(LED, !digitalRead(LED));
  server.sendHeader("Location", "/");//back to homepage again
  server.send(303);         //what does 303 mean?
}
  
