
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ArduinoOTA.h>
#include <LiquidCrystal_I2C.h>

#define COLUMS 16
#define ROWS   2

WiFiManager wifiManager;

LiquidCrystal_I2C lcd(0x38);

void setup() {
  
    //wifiManager.setConfigPortalTimeout (180);
    wifiManager.setDebugOutput (true);
    //wifiManager.resetSettings ();

    Serial.begin (9600);


    Serial.println ("connecting...");
    if (wifiManager.autoConnect ("Dupario")) {
        Serial.println ("connected ... yay!");
    } else {
        Serial.println ("connection failed, rebooting");
        ESP.restart();
    }


    
    lcd.begin(16,2);               // initialize the lcd 

 
    lcd.home ();                   // go home
    lcd.print("Hello, ARDUINO ");  
    lcd.setCursor ( 0, 1 );        // go to the next line
    lcd.print (" FORUM - fm   ");

    delay(1000);

    lcd.clear();

    /* prints static text */
    lcd.setCursor(0, 1);            //set 1-st colum & 2-nd row, 1-st colum & row started at zero
    lcd.print(F("Hello world!"));
    lcd.setCursor(0, 2);            //set 1-st colum & 3-rd row, 1-st colum & row started at zero
    lcd.print(F("Random number:"));

    ArduinoOTA.onStart([]() {
        String type;
        if (ArduinoOTA.getCommand() == U_FLASH) {
            type = "sketch";
        } else { // U_SPIFFS
            type = "filesystem";
        }

        // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
        Serial.println("Start updating " + type);
    });

    ArduinoOTA.onEnd([]() {
        Serial.println("\nEnd");
    });
    ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
        Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
    });
    ArduinoOTA.onError([](ota_error_t error) {
          Serial.printf("Error[%u]: ", error);
        if (error == OTA_AUTH_ERROR) {
            Serial.println("Auth Failed");
        } else if (error == OTA_BEGIN_ERROR) {
           Serial.println("Begin Failed");
        } else if (error == OTA_CONNECT_ERROR) {
            Serial.println("Connect Failed");
        } else if (error == OTA_RECEIVE_ERROR) {
            Serial.println("Receive Failed");
        } else if (error == OTA_END_ERROR) {
            Serial.println("End Failed");
        }
    });
    ArduinoOTA.begin();

#define THE_LED D4
    pinMode (THE_LED, OUTPUT);
}

void loop() {
    static int count=0;
    static bool lite=true;

    if (++count >= 1000) {
        count=0;
        lite = ! lite;
        digitalWrite (THE_LED, lite);
    }
    delay (1);
    ArduinoOTA.handle();
}