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
int flagForEmail = 0; //Flag to make sure mail is only send once. 0=send, 2=don't send
int value = 0; //Value from moisture sensor
//The equation for getting precentage
int max_dry = 360; //This is our 100%
int moist_precent = 0;

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
    digitalWrite(RED_LED,LOW);
    flagForEmail = 0;
  }

  // put your main code here, to run repeatedly:
   delay(3000);
   Serial.print("MOISTURE LEVEL : ");
   value=analogRead(SENSOR_PIN);
   Serial.print(value);
   Serial.print("   ");
   moist_precent = (max_dry - value) * 100 / max_dry;
   Serial.print(moist_precent);
   Serial.println("%");
  
  if(flagForEmail == 0) {
              String email_msg="<html><body>test " + (int)value;
              email_msg += "</body></html>";
              //Email Section
               Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
               String subject = "Alert! Water Leakage under tiles";
               if(gsender->Subject(subject)->Send("yanivben3@gmail.com", email_msg) {
                    Serial.println("Message send.");
                } else {
                    Serial.print("Error sending message: ");
                    Serial.println(gsender->getError());
                }
                flagForEmail = 2;
    }
}
