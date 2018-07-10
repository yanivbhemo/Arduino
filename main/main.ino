#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include "Gsender.h"

// select which pin will trigger the configuraton portal when set to LOW
// ESP-01 users please note: the only pins available (0 and 2), are shared 
// with the bootloader, so always set them HIGH at power-up
#define TRIGGER_PIN 4
#define RED_LED 15
#define SENSOR_PIN 0 // sensor input at Analog pin D4
#define YELLOW_LED 12 //Indicator led
#define INTERNAL_LED 16 //On-board red led light

int temp = 0;
int flagForEmail = 2; //Flag to make sure mail is only send once. 0=send, 2=don't send
int value = 0; //Value from moisture sensor
//The equation for getting precentage
int max_dry = 364; //This is our 100%
int moist_precent = 0;

//Extra web portal in order to configure the details of the sensor like location, address...
WiFiClient client;
ESP8266WebServer server(80); // Start server on port 80 (default for a web-browser, change to your requirements, e.g. 8080 if your Router uses port 80
                             // To access server from the outsid of a WiFi network e.g. ESP8266WebServer server(8266); and then add a rule on your Router that forwards a
                             // connection request to http://your_network_ip_address:8266 to port 8266 and view your ESP server from anywhere.
                             // Example http://g6ejd.uk.to:8266 will be directed to http://192.168.0.40:8266 or whatever IP address your router gives to this server

String Argument_Name, Clients_Response1, Clients_Response2;
void ShowClientResponse();
void HandleClient();
//----------------------------------------------------

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting");
  Serial.println("Starting");
  
  pinMode(RED_LED, OUTPUT);
  pinMode(YELLOW_LED, OUTPUT);
  pinMode(TRIGGER_PIN, INPUT);
  
  digitalWrite(RED_LED,LOW);
  digitalWrite(YELLOW_LED,LOW);  
}

void HandleClient() {
  digitalWrite(YELLOW_LED,HIGH); 
  String webpage;
  webpage =  "<html>";
   webpage += "<head><title>ESP8266 Input Example</title>";
    webpage += "<style>";
     webpage += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
    webpage += "</style>";
   webpage += "</head>";
   webpage += "<body>";
    webpage += "<h1><br>ESP8266 Server - Getting input from a client</h1>";  
    String IPaddress = WiFi.localIP().toString();
    webpage += "<form action='http://"+IPaddress+"' method='POST'>";
     webpage += "&nbsp;&nbsp;&nbsp;&nbsp;Please enter your Name:<input type='text' name='name_input'><BR>";
     webpage += "Please enter your Address:<input type='text' name='address_input'>&nbsp;<input type='submit' value='Enter'>"; // omit <input type='submit' value='Enter'> for just 'enter'
    webpage += "</form>";
   webpage += "</body>";
  webpage += "</html>";
  server.send(200, "text/html", webpage); // Send a response to the client asking for input
  if (server.args() > 0 ) { // Arguments were received
    for ( uint8_t i = 0; i < server.args(); i++ ) {
      Serial.print(server.argName(i)); // Display the argument
      Argument_Name = server.argName(i);
      if (server.argName(i) == "name_input") {
        Serial.print(" Input received was: ");
        Serial.println(server.arg(i));
        Clients_Response1 = server.arg(i);
        // e.g. range_maximum = server.arg(i).toInt();   // use string.toInt()   if you wanted to convert the input to an integer number
        // e.g. range_maximum = server.arg(i).toFloat(); // use string.toFloat() if you wanted to convert the input to a floating point number
      }
      if (server.argName(i) == "address_input") {
        Serial.print(" Input received was: ");
        Serial.println(server.arg(i));
        Clients_Response2 = server.arg(i);
        // e.g. range_maximum = server.arg(i).toInt();   // use string.toInt()   if you wanted to convert the input to an integer number
        // e.g. range_maximum = server.arg(i).toFloat(); // use string.toFloat() if you wanted to convert the input to a floating point number
      }
    }
  }
}

void ShowClientResponse() {
  String webpage;
  webpage =  "<html>";
   webpage += "<head><title>ESP8266 Input Example</title>";
    webpage += "<style>";
     webpage += "body { background-color: #E6E6FA; font-family: Arial, Helvetica, Sans-Serif; Color: blue;}";
    webpage += "</style>";
   webpage += "</head>";
   webpage += "<body>";
    webpage += "<h1><br>ESP8266 Server - This was what the client sent</h1>";
    webpage += "<p>Name received was: " + Clients_Response1 + "</p>";
    webpage += "<p>Address received was: " + Clients_Response2 + "</p>";
   webpage += "</body>";
  webpage += "</html>";
  server.send(200, "text/html", webpage); // Send a response to the client asking for input
}

void loop() {
  // is configuration portal requested?
  if ( digitalRead(TRIGGER_PIN) == HIGH ) {
    digitalWrite(RED_LED,HIGH);
    Serial.println("Reset");
    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    wifiManager.autoConnect("Noleak_001");
    //reset settings - for testing
    //wifiManager.resetSettings();

    //sets timeout until configuration portal gets turned off
    //useful to make it all retry or go to sleep
    //in seconds
    //wifiManager.setTimeout(120);

    //it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration

    WiFiManagerParameter custom_text("<p>This is just a text paragraph</p>");
    wifiManager.addParameter(&custom_text);
    
    if (!wifiManager.startConfigPortal("Noleak")) {
      Serial.println("failed to connect and hit timeout");
      delay(3000);
      //reset and try again, or maybe put it to deep sleep
      ESP.reset();
      delay(5000);
    }

    //if you get here you have connected to the WiFi
    Serial.println("connected...yeey :)");
    Serial.println("Wifi Connected Success!");
    Serial.print("NodeMCU IP Address : ");
    Serial.println(WiFi.localIP() );
    server.on("/", HandleClient); // The client connected with no arguments e.g. http:192.160.0.40/
    server.on("/result", ShowClientResponse);
    server.handleClient();
    
    digitalWrite(RED_LED,LOW);
    flagForEmail = 1;
  }

  // put your main code here, to run repeatedly:
   delay(3000);
   Serial.print("MOISTURE LEVEL : ");
   value=analogRead(SENSOR_PIN);
   Serial.print(value);
   Serial.print("   ");
   moist_precent = (max_dry - value - 2) * 100 / max_dry;
   Serial.print(moist_precent);
   Serial.println("%");
  
  if(flagForEmail == 0) {
              String email_msg="<html><body>test " + (int)value;
              email_msg += "</body></html>";
              //Email Section
               Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
               String subject = "Alert! Water Leakage under tiles";
               if(gsender->Subject(subject)->Send("yanivben3@gmail.com", email_msg)) {
                    Serial.println("Message send.");
                } else {
                    Serial.print("Error sending message: ");
                    Serial.println(gsender->getError());
                }
                flagForEmail = 2;
    }
}
