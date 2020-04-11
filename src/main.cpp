#include <Arduino.h>
#include <ArduinoOTA.h>
#include <Homie.hpp>

const int sensor_pin = D5;
unsigned long flow_count=0;
unsigned long last_seen_flow=0;
unsigned long debounce_last=0;

#define DEBOUNCE_DELAY_MS 100
#define FLOW_INTERVAL_SEC 10

ICACHE_RAM_ATTR
void flow_meter_isr () {
    unsigned int now = millis();
    if (debounce_last == 0 || (now - debounce_last) > DEBOUNCE_DELAY_MS ) {
        debounce_last = now;
        ++ flow_count;
    }

}

ICACHE_RAM_ATTR
int reset_flow_counter (String command) {
    flow_count=0;
    return 1;
}

HomieNode flowNode ("flow", "Flow", "flow");


//bool flowHandler (const HomieRange & range, const String & value) {}

unsigned long lastFlowSent=0;
void loopHandler () {
    if (millis() - lastFlowSent >= FLOW_INTERVAL_SEC * 1000UL || lastFlowSent == 0) {

        unsigned long current_flow_count = flow_count - last_seen_flow;
        last_seen_flow = flow_count;

        float flow = ((float) current_flow_count) / 2.0;

        Homie.getLogger() << "Water flow: " << flow << " l" << endl;
        flowNode.setProperty("litres").send (String(flow));

        lastFlowSent = millis();
    }
}

void setup() {
  Serial.begin(9600);

  while (!Serial) {
    ; // wait for serial port to connect. Needed for native USB port only
  }

    pinMode (sensor_pin, INPUT);
    attachInterrupt (sensor_pin, flow_meter_isr, RISING);

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


  Homie_setFirmware("flow-meter", "1.0.0"); // The underscore is not a typo! See Magic bytes

  flowNode.advertise ("litres").setName ("Litres").setDatatype ("float").setUnit ("l");

  Homie.setLoopFunction(loopHandler);

  Homie.setup();
  Homie.getLogger() << F("✔ main: Setup ready") << endl;
}

void loop() {
    ArduinoOTA.handle();
  Homie.loop();
}

// void setup() {
  
//     //wifiManager.setConfigPortalTimeout (180);
//     //wifiManager.setDebugOutput (true);
//     //wifiManager.resetSettings ();

//     Serial.begin (9600);

//     Serial.println ("Starting Flow Meter");

//     wifi_configure ();
    
//     rest.variable ("flow", &flow_count);
//     rest.variable ("mdns-configured", &mdns_configured);
//     rest.function ("reset-flow", reset_flow_counter);
//     rest.function ("reset-net", reset_networking_data);
//     rest.function ("reboot", reboot);
//     rest.set_id ("flow-meter");
//     rest.set_name ("esp8266");

//     // Start the server
//     server.begin();
//     Serial.println("Server started");


//     // mdns_configured = MDNS.begin ("flow-meter");
//     // if (!mdns_configured) {
//     //     Serial.println ("Error configuring mDNS");
//     // } else {
        
//     // }
//      // Add service to MDNS-SD
//     //MDNS.addService("http", "tcp", 80);

//     ArduinoOTA.onStart([]() {
//         String type;
//         if (ArduinoOTA.getCommand() == U_FLASH) {
//             type = "sketch";
//         } else { // U_SPIFFS
//             type = "filesystem";
//         }

//         // NOTE: if updating SPIFFS this would be the place to unmount SPIFFS using SPIFFS.end()
//         Serial.println("Start updating " + type);
//     });

//     ArduinoOTA.onEnd([]() {
//         Serial.println("\nEnd");
//     });
//     ArduinoOTA.onProgress([](unsigned int progress, unsigned int total) {
//         Serial.printf("Progress: %u%%\r", (progress / (total / 100)));
//     });
//     ArduinoOTA.onError([](ota_error_t error) {
//           Serial.printf("Error[%u]: ", error);
//         if (error == OTA_AUTH_ERROR) {
//             Serial.println("Auth Failed");
//         } else if (error == OTA_BEGIN_ERROR) {
//            Serial.println("Begin Failed");
//         } else if (error == OTA_CONNECT_ERROR) {
//             Serial.println("Connect Failed");
//         } else if (error == OTA_RECEIVE_ERROR) {
//             Serial.println("Receive Failed");
//         } else if (error == OTA_END_ERROR) {
//             Serial.println("End Failed");
//         }
//     });
//     ArduinoOTA.begin();

//     pinMode (sensor_pin, INPUT);
//     attachInterrupt (sensor_pin, flow_meter_isr, RISING);

//     Homie_setFirmware ("flow-sensor", "1.0.0");
//     Homie.setup ();
//     //pinMode (D5, OUTPUT)
// }

// void loop() {

//     if (reboot_flag) {
//         ESP.restart();
//     }

//     //MDNS.update();

//     static bool last_seen = false;

//     bool current_value = digitalRead (sensor_pin);
//     if (current_value != last_seen) {
//         ++ flow_count;
//     }
//     last_seen = current_value;

//     WiFiClient client = server.available();
//     if (client) {
//         while ( ! client.available ()) {
//             delay (1);
//         }
//         rest.handle (client);
//     }

//     ArduinoOTA.handle();

//     Homie.loop();
// }