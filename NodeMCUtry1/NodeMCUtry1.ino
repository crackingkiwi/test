//What libraries are implicitly included??
//how to replace pinMode by DDR instruction?

#include <Arduino.h>
#include <gpio.h>
#include <ESP8266WiFi.h>
#include <stddef.h>
#include <user_interface.h>


static ETSTimer timer0;
const char *ssid = "LB2.4G202PDCN";
const char *password = "zhao63148842";
const char *host = "192.168.12.45";
const static int httpPort = 80;

static void alarmfun(){
//static void IRAM_ATTR alarmfun(){
  Serial.print("Current time is: "); Serial.print(micros()); Serial.println("us");
}

// D4 is the default built-in led pin
void setup() {
  
  pinMode(D4, 1);
  os_timer_disarm(&timer0);
  os_timer_setfn(&timer0, reinterpret_cast<os_timer_func_t *>(alarmfun), NULL);
  os_timer_arm(&timer0,1000,0);
  Serial.begin(115200);
  delay(10);
  Serial.println('\n');

  WiFi.begin(ssid, password);
  Serial.print("Connecting to ");
  Serial.print(ssid); Serial.println(" ...");

  int i = 0;
  while(WiFi.status()!=WL_CONNECTED) {
    delay(1000);
    i++;
    Serial.print(i);Serial.print(' ');
  }

  Serial.println("\nConnected!");
  Serial.println("IP: "); Serial.println(WiFi.localIP());
  Serial.print("Netmask: "); Serial.println(WiFi.subnetMask());
  Serial.print("Gateway: "); Serial.println(WiFi.gatewayIP());
}

int j = 0;


void loop() {
  Serial.println(micros());
  delay(5000);
  j++;
  Serial.print("Connecting to "); Serial.println(host);

  WiFiClient client;
  if(!client.connect(host,httpPort)) {
    Serial.println("Connection failed");
  }

  //connection success
  String url = "/";
  Serial.print("Requesting URL: "); Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" + 
        "Host: " + host + "\r\n" + "Connection: close\r\n\r\n");
  delay(500);

  while(client.available()){
    String line = client.readStringUntil('\r');
    Serial.print(line);
  }
  
  Serial.println();Serial.println("closing connection");
}
