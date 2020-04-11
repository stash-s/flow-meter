
#include <WiFiManager.h>          //https://github.com/tzapu/WiFiManager WiFi Configuration Magic

WiFiManager wifiManager;

int reset_networking_data (String command) {
    wifiManager.resetSettings ();
    return 0;
}

void wifi_configure () {
    Serial.println ("connecting...");
    if (wifiManager.autoConnect ("zoo-autoconfig")) {
        Serial.println ("connected ... yay!");
    } else {
        Serial.println ("connection failed, rebooting");
        ESP.restart();
    }
}