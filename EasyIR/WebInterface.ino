//What libraries are implicitly included??
//how to replace pinMode by DDR instruction?

#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WiFiMulti.h>
//#include <ESP8266mDNS.h>
#include <ESP8266WebServer.h>
#include <FS.h>
#include <WebSocketsServer.h>

const char *ssid = "LB2.4G202PDCN";
const char *password = "zhao63148842";

#define LED 2//D4 = GPIO2/
 
ESP8266WebServer server(80);

File fsUploadFile;

void handleRoot()
{
  //server.send(200, "text/html", "<form action=\"/LED\" method=\"POST\">"
  //"<input type=\"submit\" value=\"Toggle LED\"></form>");
  /*
  server.send(200, "text/html", 
  "<form action=\"/initIRreceiver\" method=\"POST\">" 
  "<input type=\"submit\" value=\"Receiver Start\">"  
  "</form>" 
  "<p>Get the IR code history: </p>"
  "<form action=\"/Codehistory\" method=\"POST\">"
  "<input type=\"submit\" value=\"Print_history\">"
  "</form>"
  "<p>Upload a file: </p>"
  "<form action=\"/upload\" method=\"POST\" enctype=\"multipart/form-data\">"
  "<input type=\"file\" name=\"name\">"
  "<input class=\"button\" type=\"submit\" value=\"Upload\">"
  "</form>" 
  );
  */
  File mainpageFile = SPIFFS.open("index.html", "r");
  server.streamFile(mainpageFile,"text/html");
}

bool handleFileRead(String path)
{
  //char message[128];
  //strcpy(message, "Try to find:"); strcat(message, path);
  //Serial.println(message);
  Serial.println("Try to find: " + path);
  if(SPIFFS.exists(path))
  {
    File file=SPIFFS.open(path, "r");
    if(path.endsWith(".html"))server.streamFile(file,"text/html");
    else if(path.endsWith(".png"))server.streamFile(file,"image/png");
    file.close();
    return 1;
  }
  Serial.println("File Not Found");;
  return 0;
}

void handleNotFound()
{
  if(!handleFileRead(server.uri()))
  {
    server.send(404, "text/plain", "404: Not Foud");
  }
}

void handleInitReceiver()
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

void handleFileUpload()
{
  Serial.println("Calling handleFileUpload");
  //server.send(200);
  HTTPUpload& upload = server.upload();
  if(upload.status==UPLOAD_FILE_START)
  {
    String filename = "/" + upload.filename;
    Serial.print("Uploading: "); Serial.println(filename);
    fsUploadFile = SPIFFS.open(filename, "w");
  }
  else if(upload.status==UPLOAD_FILE_WRITE)
  {
    if(fsUploadFile)
    {
      fsUploadFile.write(upload.buf, upload.currentSize);
    }
  }
  else if(upload.status==UPLOAD_FILE_END)
  {
    if(fsUploadFile) 
    {
      fsUploadFile.close();
      Serial.println("Uploaded");
      server.send(303);
    }
    else
    {
      server.send(500,"text/plain", "500: couldn't creat file");
    }
  }
}
void StartWifi_afterSerial()
{
  int i=0;
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
}

void setup_Server()
{
  server.on("/", HTTP_GET, handleRoot);
  server.onNotFound(handleNotFound);
  
  server.on("/initIRreceiver", HTTP_POST, handleInitReceiver);
  server.on("/Codehistory", HTTP_POST, handleCodehistory);
  server.on("/upload", HTTP_POST, [](){server.send(200);}, handleFileUpload);
}

void setup() 
{  
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  pinMode(LED,1);
  
  StartWifi_afterSerial();

  //if(MDNS.begin("nodemcu"))//notworking, don't know why...
  //{
  //  Serial.println("mDNS reponder started");
  //}

  setup_Server();
  
  SPIFFS.begin();
  server.begin();
  
  Serial.println("HTTP server started");
  //digitalWrite(LED, 1);
}

void loop()
{
  server.handleClient();
  wait_for_signal(); 
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
  
