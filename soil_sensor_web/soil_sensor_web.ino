#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
#include <ESP8266mDNS.h>
#include <DNSServer.h>
#include <ESP8266HTTPUpdateServer.h>
#include <WiFiManager.h>
#include <WiFiClientSecure.h>
#include "DHT.h"
#include "SimpleTimer.h"
String readString;

const char* host = "script.google.com";
const int httpsPort = 443;
// Use WiFiClientSecure class to create TLS connection
WiFiClientSecure client;

// SHA1 fingerprint of the certificate, don't care with your GAS service
const char* fingerprint = "";
String GAS_ID = "";   // Replace by your GAS service id                  

#define D0 16
#define D1 5
#define D2 4
#define D3 0
#define D4 2
#define D5 14
#define D6 12
#define D7 13
#define D8 15

ESP8266WebServer httpServer(80);
ESP8266HTTPUpdateServer httpUpdater;
#define host2 "ESP8266-server"

SimpleTimer timer;

//DHT configs
#define DHTTYPE DHT22   // DHT 22  (AM2302), AM2321
//uint8_t DHTPin = 2;
#define DHTPin D1
#define Relay1pin D2
#define Relay2pin D3
#define Relay3pin D4
#define Soil1pin D5
#define Soil2pin D6
int sensor_analog_pin= A0;

DHT dht(DHTPin, DHTTYPE);
float Temperature;
float Humidity;

//soil sensor configs
//uint8_t Relay1pin = 4;
bool Relay1status = LOW;
//uint8_t Relay2pin = 5;
bool Relay2status = LOW;
//uint8_t Relay3pin = 6;
bool Relay3status = LOW;
int Soil_sensor1, Soil_sensor2;
bool automation;

bool first_time = true;


void setup() {
  Serial.begin(115200);
  delay(100);
  WiFiManager wifiManager;
  wifiManager.setTimeout(30);
  wifiManager.autoConnect("AutoConnectAP");
  /*
    if (!wifiManager.startConfigPortal("AutoConnectAP")) {
    Serial.println("failed to connect and hit timeout");
    delay(3000);
    //reset and try again, or maybe put it to deep sleep
    ESP.reset();
    delay(5000);
    }
  */
  Serial.println("");
  pinMode(Relay1pin, OUTPUT);
  pinMode(Relay2pin, OUTPUT);
  digitalWrite(Relay1pin, LOW);
  digitalWrite(Relay2pin, LOW);
  pinMode(DHTPin, INPUT);
  if (MDNS.begin(host2) != true ) {
    Serial.println("starting mdns failed");
  }

  httpUpdater.setup(&httpServer);
  httpServer.begin();

  MDNS.addService("http", "tcp", 80);
  Serial.printf("HTTPUpdateServer ready! Open http://%s.local/update in your browser\n", host2);
  Serial.println("");
  Serial.println("WiFi connected..!");
  Serial.print("Got IP: ");  Serial.println(WiFi.localIP());

  httpServer.on("/", handle_OnConnect);
  httpServer.on("/relay1on", handle_relay1on);
  httpServer.on("/relay1off", handle_relay1off);
  httpServer.on("/relay2on", handle_relay2on);
  httpServer.on("/relay2off", handle_relay2off);
  httpServer.on("/relay3on", handle_relay3on);
  httpServer.on("/relay3off", handle_relay3off);
  httpServer.on("/automation_on", handle_automation_on);
  httpServer.on("/automation_off", handle_automation_off);
  httpServer.onNotFound(handle_NotFound);

  
  Serial.println("HTTP server started");
  //send_mail("bigdeli.ali3@gmail.com", "testing another");
  timer.setInterval(5000, read_sensor_value);
}
void loop() {

  httpServer.handleClient();
  MDNS.update();
  timer.run();
  conditions ();
  relay_actions();

}

void conditions() {
  if (automation == true) {
    if ((Soil_sensor1 <= 40 ) || (Soil_sensor2 <= 40 )) {
      //starting water pump
      Relay1status = HIGH;
    }
    else {
      //stoping water pump
      Relay1status = LOW;
    }
    if (Temperature >= 29) {
      //starting fan
      Relay2status = HIGH;
    }
    else {
      //stoping fan
      Relay2status = LOW;
    }
    if (Temperature <= 28) {
      //starting heater
      Relay3status = HIGH;
    }
    else {
      //stoping heater
      Relay3status = LOW;
    }
  }
}

void relay_actions() {
  if (Relay1status)
  {
    digitalWrite(Relay1pin, HIGH);
  }
  else
  {
    digitalWrite(Relay1pin, LOW);
  }

  if (Relay2status)
  {
    digitalWrite(Relay2pin, HIGH);
  }
  else
  {
    digitalWrite(Relay2pin, LOW);
  }
  if (Relay3status)
  {
    digitalWrite(Relay3pin, HIGH);
  }
  else
  {
    digitalWrite(Relay3pin, LOW);
  }
}
void handle_relay1on() {
  Relay1status = HIGH;
  Serial.println("GPIO4 Status: ON");
  httpServer.send(200, "text/html", SendHTML(Temperature, Humidity, Soil_sensor1, Soil_sensor2, Relay1status, Relay2status, Relay3status, automation));
}

void handle_relay1off() {
  Relay1status = LOW;
  Serial.println("GPIO4 Status: OFF");
  httpServer.send(200, "text/html", SendHTML(Temperature, Humidity, Soil_sensor1, Soil_sensor2, Relay1status, Relay2status, Relay3status, automation));
}

void handle_relay2on() {
  Relay2status = HIGH;
  Serial.println("GPIO5 Status: ON");
  httpServer.send(200, "text/html", SendHTML(Temperature, Humidity, Soil_sensor1, Soil_sensor2, Relay1status, Relay2status, Relay3status, automation));
}

void handle_relay2off() {
  Relay2status = LOW;
  Serial.println("GPIO5 Status: OFF");
  httpServer.send(200, "text/html", SendHTML(Temperature, Humidity, Soil_sensor1, Soil_sensor2, Relay1status, Relay2status, Relay3status, automation));
}

void handle_relay3on() {
  Relay3status = HIGH;
  Serial.println("GPIO5 Status: ON");
  httpServer.send(200, "text/html", SendHTML(Temperature, Humidity, Soil_sensor1, Soil_sensor2, Relay1status, Relay2status, Relay3status, automation));
}

void handle_relay3off() {
  Relay3status = LOW;
  Serial.println("GPIO5 Status: OFF");
  httpServer.send(200, "text/html", SendHTML(Temperature, Humidity, Soil_sensor1, Soil_sensor2, Relay1status, Relay2status, Relay3status, automation));
}

void handle_automation_on() {
  automation = HIGH;
  Serial.println("automation Status: ON");
  httpServer.send(200, "text/html", SendHTML(Temperature, Humidity, Soil_sensor1, Soil_sensor2, Relay1status, Relay2status, Relay3status, automation));
}

void handle_automation_off() {
  automation = LOW;
  Serial.println("automation Status: OFF");
  httpServer.send(200, "text/html", SendHTML(Temperature, Humidity, Soil_sensor1, Soil_sensor2, Relay1status, Relay2status, Relay3status, automation));
}


void handle_OnConnect() {
  float Temperature_t = Temperature;
  float Humidity_t = Humidity;
  float Soil_sensor1_t = Soil_sensor1;
  float Soil_sensor2_t = Soil_sensor2;
  //Soil_sensor2 = read_sensor_value2();
  //if (first_time) {
  //Relay1status = LOW;
  //Relay2status = LOW;
  //Serial.println("GPIO4 Status: OFF | GPIO5 Status: OFF");
  //first_time = false;
  //}

  //server.send(200, "text/html", SendHTML(LED1status,LED2status));
  httpServer.send(200, "text/html", SendHTML(Temperature_t, Humidity_t, Soil_sensor1_t, Soil_sensor2_t, Relay1status, Relay2status, Relay3status, automation));
}

void handle_NotFound() {
  httpServer.send(404, "text/plain", "Not found");
}


void read_sensor_value() {  
  temp_hu();
  soil_sensors();
  sendData(Temperature, Humidity, Soil_sensor1, Soil_sensor2);
}

void soil_sensors() {
  Serial.println("turn on the module Soil_sensor1");
  digitalWrite(Soil1pin, HIGH);
  delay(10);
  Soil_sensor1 = analogRead(sensor_analog_pin);
  Soil_sensor1 = map(Soil_sensor1, 1024, 400, 0, 100);
  Serial.print("Soil_sensor1 Mositure : ");
  Serial.print(Soil_sensor1);
  Serial.println("%");
  Serial.println("turn off the module Soil_sensor1");
  delay(10);
  digitalWrite(Soil1pin, LOW);
  delay(100);
  Serial.println("turn on the module Soil_sensor2");
  digitalWrite(Soil2pin, HIGH);
  delay(10);
  Soil_sensor2 = analogRead(sensor_analog_pin);
  Soil_sensor2 = map(Soil_sensor2, 1024, 400, 0, 100);
  Serial.print("Soil_sensor2 Mositure : ");
  Serial.print(Soil_sensor2);
  Serial.println("%");  
  delay(10);
  Serial.println("turn off the module Soil_sensor2");
  digitalWrite(Soil2pin, LOW);
  /*
    Soil_sensor1 = random(10, 60);
    Soil_sensor2 = random(30, 80);
    Serial.print("soil sensor 1: ");
    Serial.println(Soil_sensor1);
    Serial.print("soil sensor 2: ");
    Serial.println(Soil_sensor2);
  */
}


void temp_hu() {
  //Humidity = dht.readHumidity();
  //Temperature = dht.readTemperature();
  Humidity = random(30, 80);
  Temperature = random(10, 40);
  if (isnan(Humidity) || isnan(Temperature)) {
    Serial.println("Failed to read from DHT sensor!");
  }
  else {
    Serial.print("humidity is: ");
    Serial.println(Humidity);
    Serial.print("temp is: ");
    Serial.println(Temperature);
  }
  //delay(1000);
}

// Function for Send data into Google Spreadsheet
void sendData(int tem, int hum,int soil1,int soil2)
{
  Serial.print("connecting to ");
  Serial.println(host);
  if (!client.connect(host, httpsPort)) {
    Serial.println("connection failed");
    return;
  }

  if (client.verify(fingerprint, host)) {
  Serial.println("certificate matches");
  } else {
  Serial.println("certificate doesn't match");
  }
  String string_temperature =  String(tem, DEC); 
  String string_humidity =  String(hum, DEC); 
  String string_soil1 =  String(soil1, DEC); 
  String string_soil2 =  String(soil2, DEC); 
  String url = "/macros/s/" + GAS_ID + "/exec?temperature=" + string_temperature + "&humidity=" + string_humidity + "&soil_sensor1=" + string_soil1 + "&soil_sensor2=" + string_soil2;
  Serial.print("requesting URL: ");
  Serial.println(url);

  client.print(String("GET ") + url + " HTTP/1.1\r\n" +
         "Host: " + host + "\r\n" +
         "User-Agent: BuildFailureDetectorESP8266\r\n" +
         "Connection: close\r\n\r\n");

  Serial.println("request sent");
  while (client.connected()) {
  String line = client.readStringUntil('\n');
  if (line == "\r") {
    Serial.println("headers received");
    break;
  }
  }
  String line = client.readStringUntil('\n');
  if (line.startsWith("{\"state\":\"success\"")) {
  Serial.println("esp8266/Arduino CI successfull!");
  } else {
  Serial.println("esp8266/Arduino CI has failed");
  }
  Serial.println("reply was:");
  Serial.println("==========");
  Serial.println(line);
  Serial.println("==========");
  Serial.println("closing connection");
} 


String SendHTML(float Temperaturestat, float Humiditystat, float Soil_sensor1, float Soil_sensor2, uint8_t relay1stat, uint8_t relay2stat, uint8_t relay3stat, bool automate) {
  String ptr = "<!DOCTYPE html> <html>\n";
  ptr += "<head><meta name=\"viewport\" content=\"width=device-width, initial-scale=1.0, user-scalable=no\">\n";
  ptr += "<title>ESP32 Weather Report</title>\n";
  ptr += "<style>html { font-family: Helvetica; display: inline-block; margin: 0px auto; text-align: center;}\n";
  ptr += "body{margin-top: 50px;} h1 {color: #444444;margin: 50px auto 30px;} h3 {color: #444444;margin-bottom: 50px;}\n";
  ptr += "p {font-size: 24px;color: #444444;margin-bottom: 10px;}\n";
  ptr += ".button {display: block;width: 80px;background-color: #3498db;border: none;color: white;padding: 13px 30px;text-decoration: none;font-size: 25px;margin: 0px auto 35px;cursor: pointer;border-radius: 4px;}\n";
  ptr += ".button-on {background-color: #3498db;}\n";
  ptr += ".button-on:active {background-color: #2980b9;}\n";
  ptr += ".button-off {background-color: #34495e;}\n";
  ptr += ".button-off:active {background-color: #2c3e50;}\n";
  ptr += "p1 {font-size: 14px;color: #888;margin-bottom: 10px;}\n";
  ptr += "</style>\n";
  ptr += "<script>\n";
  ptr += "setInterval(loadDoc,200);\n";
  ptr += "function loadDoc() {\n";
  ptr += "var xhttp = new XMLHttpRequest();\n";
  ptr += "xhttp.onreadystatechange = function() {\n";
  ptr += "if (this.readyState == 4 && this.status == 200) {\n";
  ptr += "document.getElementById(\"webpage\").innerHTML =this.responseText}\n";
  ptr += "};\n";
  ptr += "xhttp.open(\"GET\", \"/\", true);\n";
  ptr += "xhttp.send();\n";
  ptr += "return false;}\n";
  ptr += "</script>\n";
  ptr += "<script>";
  ptr += "function send_status(status){\n ";
  ptr += "var xhttp = new XMLHttpRequest();\n";
  ptr += "xhttp.open(\"GET\", '/' + status, true);\n";
  ptr += "xhttp.send();\n";
  ptr += "\n}";
  ptr += "</script>\n";
  ptr += "</head>\n";
  ptr += "<body>\n";
  ptr += "<div id=\"webpage\">\n";
  ptr += "<h1>ESP8266 Smart Garden Report</h1>\n";

  ptr += "<p>Temperature: ";
  ptr += (int)Temperaturestat;
  ptr += "°C</p>";
  ptr += "<p>Humidity: ";
  ptr += (int)Humiditystat;
  ptr += "%</p>";


  ptr += "<p>Soil sensor 1: ";
  ptr += (int)Soil_sensor1;
  //ptr += "°C</p>";
  ptr += "%</p>";
  ptr += "<p>Soil sensor 2: ";
  ptr += (int)Soil_sensor2;
  ptr += "%</p>";

  if (automate)
  {
    //ptr += "<p1>Relay1 Status: ON</p><a class=\"button button-off\" href=\"/relay1off\">OFF</a>\n";
    ptr += "<p1>automation Status: ON</p><a class=\"button button-off\" onclick=\"return send_status('automation_off');\">OFF</a>\n";
  }
  else
  {
    //ptr += "<p1>Relay1 Status: OFF</p><a class=\"button button-on\" href=\"/relay1on\">ON</a>\n";
    ptr += "<p1>automation Status: OFF</p><a class=\"button button-on\" onclick=\"return send_status('automation_on');\">ON</a>\n";
  }

  if (relay1stat)
  {
    //ptr += "<p1>Relay1 Status: ON</p><a class=\"button button-off\" href=\"/relay1off\">OFF</a>\n";
    ptr += "<p1>Relay1 Status: ON</p><a class=\"button button-off\" onclick=\"return send_status('relay1off');\">OFF</a>\n";
  }
  else
  {
    //ptr += "<p1>Relay1 Status: OFF</p><a class=\"button button-on\" href=\"/relay1on\">ON</a>\n";
    ptr += "<p1>Relay1 Status: OFF</p><a class=\"button button-on\" onclick=\"return send_status('relay1on');\">ON</a>\n";
  }

  if (relay2stat)
  {
    //ptr += "<p1>Relay2 Status: ON</p><a class=\"button button-off\" href=\"/relay2off\">OFF</a>\n";
    ptr += "<p1>Relay2 Status: ON</p><a class=\"button button-off\" onclick=\"return send_status('relay2off');\">OFF</a>\n";
  }
  else
  {
    //ptr += "<p1>Relay2 Status: OFF</p><a class=\"button button-on\" href=\"/relay2on\">ON</a>\n";
    ptr += "<p1>Relay2 Status: OFF</p><a class=\"button button-on\" onclick=\"return send_status('relay2on');\">ON</a>\n";
  }

  if (relay3stat)
  {
    //ptr += "<p1>Relay2 Status: ON</p><a class=\"button button-off\" href=\"/relay2off\">OFF</a>\n";
    ptr += "<p1>Relay3 Status: ON</p><a class=\"button button-off\" onclick=\"return send_status('relay3off');\">OFF</a>\n";
  }
  else
  {
    //ptr += "<p1>Relay2 Status: OFF</p><a class=\"button button-on\" href=\"/relay2on\">ON</a>\n";
    ptr += "<p1>Relay3 Status: OFF</p><a class=\"button button-on\" onclick=\"return send_status('relay3on');\">ON</a>\n";
  }


  ptr += "</div>\n";
  ptr += "</body>\n";
  ptr += "</html>\n";
  return ptr;
}
