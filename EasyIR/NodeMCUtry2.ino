//What libraries are implicitly included??
//how to replace pinMode by DDR instruction?

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
//#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>

const char *ssid = "LB2.4G202PDCN";
const char *password = "zhao63148842";

#define LED D4
 
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

  //if(MDNS.begin("nodemcu"))//notworking, don't know why...
  //{
  //  Serial.println("mDNS reponder started");
  //}
  
  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);

  server.on("/initIRreceiver", HTTP_POST, handleinitIRreceiver);
  server.on("/Codehistory", HTTP_POST, handleCodehistory);

  server.begin();
  Serial.println("HTTP server started");
  digitalWrite(LED, 1);
}

void loop()
{
  server.handleClient();
  wait_for_signal();
  
}

void handleRoot()
{
  //server.send(200, "text/html", "<form action=\"/LED\" method=\"POST\">"
  //"<input type=\"submit\" value=\"Toggle LED\"></form>");
  server.send(200, "text/html", 
  "<form action=\"/initIRreceiver\" method=\"POST\">" 
  "<input type=\"submit\" value=\"Receiver Start\">"  
  "</form>" 
  "<p>Get the IR code history: </p>"
  "<form action=\"/Codehistory\" method=\"POST\">"
  "<input type=\"submit\" value=\"Print_history\">"
  "</form>"
  );
}

void handleNotFound()
{
  server.send(404, "text/plain", "404: Not Foud");
}


void handleinitIRreceiver()
{
  digitalWrite(LED, 0);
  server.sendHeader("Location", "/");
  init_receiver();
  Serial.println("Receiver started");
  server.send(303);
  //server.send(303, "text/plain", "Receiver started.");notworking!!
}

void handleCodehistory()
{
  hex_data[hex_data_length]=NULL;
  Serial.println(hex_data);
  server.send(200, "text/plain", hex_data);
}
  /*


  
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
}*/
  
