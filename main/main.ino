#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include "Gsender.h"

// select wich pin will trigger the configuraton portal when set to LOW
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

   Serial.print("MOISTURE LEVEL : ");
   value=analogRead(SENSOR_PIN);
   moist_precent = (value * 100) / max_dry;
   Serial.println(moist_precent + "%");
  
  if(flagForEmail == 0) {
              //Email Section
               Gsender *gsender = Gsender::Instance();    // Getting pointer to class instance
               String subject = "Alert! Water Leakage under tiles at Amnon's smart home";
               if(gsender->Subject(subject)->Send("yanivben3@gmail.com", "<h1><font color='red'>Please Be Advised!</font></h1><h2>There is a water leakage under your tiles.<br>Please fix it quickly.</h2>")) {
                    Serial.println("Message send.");
                } else {
                    Serial.print("Error sending message: ");
                    Serial.println(gsender->getError());
                }
                flagForEmail = 2;
    }
}
