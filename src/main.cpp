
#include <ESP8266WiFi.h>          //ESP8266 Core WiFi Library (you most likely already have this in your sketch)
#include <DNSServer.h>            //Local DNS Server used for redirecting all requests to the configuration portal
#include <ESP8266WebServer.h>     //Local WebServer used to serve the configuration portal
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic
#include <ArduinoOTA.h>
#include <aREST.h>


WiFiManager wifiManager;

// Create aREST instance
aREST rest = aREST();

// The port to listen for incoming TCP connections
#define LISTEN_PORT           80

// Create an instance of the server
WiFiServer server(LISTEN_PORT);

const int sensor_pin = D7;
unsigned long flow_count=0;

ICACHE_RAM_ATTR
void flow_meter_isr () {
    ++ flow_count;
}

ICACHE_RAM_ATTR
int reset_flow_counter (String command) {
    flow_count=0;
    return 1;
}

void setup() {
  
    //wifiManager.setConfigPortalTimeout (180);
    //wifiManager.setDebugOutput (true);
    //wifiManager.resetSettings ();

    Serial.begin (9600);

    Serial.println ("connecting...");
    if (wifiManager.autoConnect ("Dupario")) {
        Serial.println ("connected ... yay!");
    } else {
        Serial.println ("connection failed, rebooting");
        ESP.restart();
    }
    
    rest.variable ("flow", &flow_count);
    rest.function ("reset", reset_flow_counter);
    rest.set_id ("dupario");
    rest.set_name ("esp8266");

    // Start the server
    server.begin();
    Serial.println("Server started");

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

    pinMode (sensor_pin, INPUT);
    attachInterrupt (sensor_pin, flow_meter_isr, CHANGE);

    //pinMode (D5, OUTPUT)
}

void loop() {

    static bool last_seen = false;

    bool current_value = digitalRead (sensor_pin);
    if (current_value != last_seen) {
        ++ flow_count;
    }
    last_seen = current_value;

    WiFiClient client = server.available();
    if (client) {
        while ( ! client.available ()) {
            delay (1);
        }
        rest.handle (client);
    }

    ArduinoOTA.handle();
}