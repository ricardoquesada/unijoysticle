/****************************************************************************
http://retro.moe/unijoysticle

Copyright 2016 Ricardo Quesada

Licensed under the Apache License, Version 2.0 (the "License");
you may not use this file except in compliance with the License.
You may obtain a copy of the License at

    http://www.apache.org/licenses/LICENSE-2.0

Unless required by applicable law or agreed to in writing, software
distributed under the License is distributed on an "AS IS" BASIS,
WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
See the License for the specific language governing permissions and
limitations under the License.
****************************************************************************/

// based on http://www.esp8266.com/viewtopic.php?f=29&t=2222


#define UNIJOYSTICLE_VERSION "v0.4.1"

// 1: try to connect to WPS. If it fails create the AP network
// 0: Create the AP network without trying WPS
#define TRY_WPS 0

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266mDNS.h>

extern "C" {
  #include "user_interface.h"
}

static const char* ssid_ap = "unijoysticle";       // SSID for Access Point
static const char* ssid_sta = "some_ssid";         // EDIT: put your SSID here
static const char* pass_sta = "thepassword";       // EDIT: put your password here

enum {
    // possible errors. Use nubmer >=2
    ERROR_CANNOT_CONNECT = 2,
    ERROR_MDNS_FAIL = 3,
};

static const unsigned int localPort = 6464;    // local port to listen for UDP packets
static IPAddress __ipAddress;                   // local IP Address
static bool __ap_mode = true;                   // whether or not it is in AP mode


static MDNSResponder mdns;                     // announce Joystick service

static const int INTERNAL_LED = D0; // Amica has two internals LEDs: D0 and D4
static const int pinsPort0[] = {D0, D1, D2, D3, D4};
static const int pinsPort1[] = {D5, D6, D7, D8, RX};
static const int TOTAL_PINS = sizeof(pinsPort0) / sizeof(pinsPort0[0]);

static byte packetBuffer[512];             //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
static WiFiUDP Udp;

void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(115200);
    Serial.setDebugOutput(1);

    delay(500);

    Serial.println("\n*** UniJoystiCle " UNIJOYSTICLE_VERSION " ***\n");

    // setting up Station AP
    setupWiFi();
    delay(500);
    printWifiStatus();

    Serial.println("Connected to wifi");
    Serial.print("Udp server started at port ");
    Serial.println(localPort);

    delay(2000);

    Udp.begin(localPort);

    if (mdns.begin("unijoysticle", __ipAddress)) {
        Serial.print("MDNS responder started in:");
        Serial.print(__ipAddress);
        Serial.print(" / ");
        Serial.println(localPort);
    }
    else
    {
        fatalError(ERROR_MDNS_FAIL);
    }

    // advertize mDNS service
    mdns.addService("unijoysticle", "udp", localPort);

    for (int i=0; i<TOTAL_PINS; i++)
    {
        pinMode(pinsPort0[i], OUTPUT);
        digitalWrite(pinsPort0[i], LOW);
        pinMode(pinsPort1[i], OUTPUT);
        digitalWrite(pinsPort1[i], LOW);
    }
}

void loop()
{
    int noBytes = Udp.parsePacket();
    if (noBytes == 0)
        return;

    String received_command = "";

    // byte 0:
    // LSB Nibble: Control Port: 0 or 1
    // MSB Nibble: 0 Joystick value
    //             1 Mouse (Not supported yet)
    //             2 Paddle (Not supported yet)
    //
    // byte 1:
    // Joystick  bit 0: up
    //           bit 1: down
    //           bit 2: left
    //           bit 3: right
    //           bit 4: fire
    if (noBytes == 2)
    {
        // DEBUG: turn on internal LED
        // digitalWrite(INTERNAL_LED, LOW);

        // Serial.print(millis() / 1000);
        // Serial.print(":Packet of ");
        // Serial.print(noBytes);
        // Serial.print(" received from ");
        // Serial.print(Udp.remoteIP());
        // Serial.print(":");
        // Serial.println(Udp.remotePort());
        // We've received a packet, read the data from it
        Udp.read(packetBuffer,noBytes); // read the packet into the buffer

        const int* pins;
        if (packetBuffer[0] == 0)
            pins = pinsPort0;
        else
            pins = pinsPort1;

        for (int i=0; i<TOTAL_PINS; i++)
        {
            if (packetBuffer[1] & (1<<i))
                digitalWrite(pins[i], HIGH);
            else
                digitalWrite(pins[i], LOW);
        }

        // Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        // Udp.write("#IP of ESP8266#");
        // Udp.println(WiFi.localIP());
        // Udp.endPacket();

        // Serial.println(received_command);
        // Serial.println();

        // DEBUG: turn off internal LED
        // digitalWrite(INTERNAL_LED, HIGH);
    }
    else if (noBytes == 4)
    {
        Udp.read(packetBuffer,noBytes); // read the packet into the buffer

        // packetBuffer[0] = version
        // packetBuffer[1] = ports enabled
        // packetBuffer[2] = joy1
        // packetBuffer[3] = joy2

        if (packetBuffer[0] == 2) {
            // joy 1 enabled ?
            if (packetBuffer[1] & 0x1) {
                for (int i=0; i<TOTAL_PINS; i++)
                {
                    if (packetBuffer[2] & (1<<i))
                        digitalWrite(pinsPort0[i], HIGH);
                    else
                        digitalWrite(pinsPort0[i], LOW);
                }
            }

            // joy 2 enabled ?
            if (packetBuffer[1] & 0x2) {
                for (int i=0; i<TOTAL_PINS; i++)
                {
                    if (packetBuffer[3] & (1<<i))
                        digitalWrite(pinsPort1[i], HIGH);
                    else
                        digitalWrite(pinsPort1[i], LOW);
                }
            }
        }
    }
    else 
    {
      Serial.println("Invalid packet");
    }
}

static void fatalError(int times)
{
    Serial.println("Fatal error. Reboot please");
    pinMode(INTERNAL_LED, OUTPUT);
    while(1) {
        // report error
        for (int i=0; i<times; i++) {
            delay(400);
            digitalWrite(INTERNAL_LED, LOW);
            delay(400);
            digitalWrite(INTERNAL_LED, HIGH);
        }
        delay(500);
    }
}

static void setupWiFi()
{
    __ap_mode = false;

#if TRY_WPS
    if (!tryWPS()) {
#else
    if (true) {
#endif
        delay(500);
        __ap_mode = true;
        Serial.print("[Creating AP]");
        WiFi.mode(WIFI_AP);
        delay(500);

        uint8_t mac[WL_MAC_ADDR_LENGTH];
        WiFi.softAPmacAddress(mac);
        char buf[50];
        memset(buf, 0, sizeof(buf)-1);
        snprintf(buf, sizeof(buf)-1, "%s-%x%x%x",
                 ssid_ap,
                 mac[WL_MAC_ADDR_LENGTH-3],
                 mac[WL_MAC_ADDR_LENGTH-2],
                 mac[WL_MAC_ADDR_LENGTH-1]);

        bool success = false;
        while(!success) {
            if ((success=WiFi.softAP(buf))) {
                Serial.print("SSID:");
                Serial.println(buf);
            } else {
                Serial.println("Failed to create AP");
                delay(1000);
            }
        }
    }
}


static bool tryWPS()
{
    bool wpsSuccess = false;
    Serial.println("WPS config start");
    WiFi.mode(WIFI_STA);
    delay(1000);
    // reading data from EPROM, last saved credentials
//    WiFi.begin(WiFi.SSID().c_str(),WiFi.psk().c_str()); 
//    WiFi.begin(ssid_sta, pass_sta);
    WiFi.begin("","");
    delay(1000);
    wpsSuccess = WiFi.isConnected();
    if (!wpsSuccess) {
        Serial.println("Failed to connect using saved credentials. Trying WPS...");
        wpsSuccess = WiFi.beginWPSConfig();
        if(wpsSuccess) {
            // in case of a timeout we might have an empty ssid
            String newSSID = WiFi.SSID();
            if(newSSID.length() > 0) {
                Serial.printf("Connected to SSID '%s'\n", newSSID.c_str());
                delay(500);
            } else {
                Serial.println("WPS failed");
                wpsSuccess = false;
            }
        }

        if (!wpsSuccess) {
            // Issue #1845: https://github.com/esp8266/Arduino/issues/1845
            delay(500);
            wifi_wps_disable();
        }
    }
    return wpsSuccess;
}

static void printWifiStatus()
{
    if (__ap_mode) {
        Serial.print("AP Station #: ");
        Serial.println(WiFi.softAPgetStationNum());
        // print your WiFi shield's IP address:
        __ipAddress = WiFi.softAPIP();
        Serial.print("Local IP Address: ");
        Serial.println(__ipAddress);
    }
    else
    {
        // print the SSID of the network you're attached to:
        Serial.print("SSID: ");
        Serial.println(WiFi.SSID());
        // print your WiFi shield's IP address:
        __ipAddress = WiFi.localIP();
        Serial.print("Local IP Address: ");
        Serial.println(__ipAddress);
    }
}
