#include <ESP8266WiFi.h>          //https://github.com/esp8266/Arduino

//needed for library
#include <ESP8266WebServer.h>
#include <DNSServer.h>
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager
#include "Gsender.h"

// select wich pin will trigger the configuraton portal when set to LOW
// ESP-01 users please note: the only pins available (0 and 2), are shared 
// with the bootloader, so always set them HIGH at power-up
#define TRIGGER_PIN 16
#define RED_LED 15

int temp = 0;
int flagForEmail = 2; //Flag to make sure mail is only send once.

void setup() {
  // put your setup code here, to run once:
  Serial.begin(115200);
  Serial.println("Starting");
  Serial.println("Starting");
  
  pinMode(RED_LED, OUTPUT);
  digitalWrite(RED_LED,LOW);
  pinMode(TRIGGER_PIN, INPUT);
}


void loop() {
  // is configuration portal requested?
  if ( digitalRead(TRIGGER_PIN) == HIGH ) {
    digitalWrite(RED_LED,HIGH);
    Serial.println("Reset");
    //WiFiManager
    //Local intialization. Once its business is done, there is no need to keep it around
    WiFiManager wifiManager;
    wifiManager.autoConnect("Yaniv");
    //reset settings - for testing
    //wifiManager.resetSettings();

    //sets timeout until configuration portal gets turned off
    //useful to make it all retry or go to sleep
    //in seconds
    //wifiManager.setTimeout(120);

    //it starts an access point with the specified name
    //here  "AutoConnectAP"
    //and goes into a blocking loop awaiting configuration

    //WITHOUT THIS THE AP DOES NOT SEEM TO WORK PROPERLY WITH SDK 1.5 , update to at least 1.5.1
    //WiFi.mode(WIFI_STA);
    
    if (!wifiManager.startConfigPortal("Yaniv")) {
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
                flagForEmail = flagForEmail + 1;
            }
}
