/****************************************************************************
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

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <WiFiUDP.h>
#include <ESP8266mDNS.h>

extern "C" {  //required for read Vdd Voltage
#include "user_interface.h"
    // uint16 readvdd33(void);
}

int status = WL_IDLE_STATUS;
const char* ssid = "queque2";           // my WiFi network
const char* pass = "locopajaro";        // my super secret password. Use to join my WiFi network.
const unsigned int localPort = 6464;    // local port to listen for UDP packets

MDNSResponder mdns;                     // announce Joystick service

// believe or not, they are not ordered
static const int pinsPort0[] = {D1, D2, D3, D4, D5};
static const int pinsPort1[] = {D6, D7, D8, D9, D10};
static const int TOTAL_PINS = sizeof(pinsPort0) / sizeof(pinsPort0[0]);

byte packetBuffer[512];             //buffer to hold incoming and outgoing packets

// A UDP instance to let us send and receive packets over UDP
WiFiUDP Udp;

void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(9600);

    // setting up Station AP
    WiFi.begin(ssid, pass);

    // Wait for connect to AP
    Serial.print("[Connecting]");
    Serial.print(ssid);
    int tries=0;
    while (WiFi.status() != WL_CONNECTED) {
        delay(500);
        Serial.print(".");
        tries++;
        if (tries > 30) {
            Serial.println("Could not connect. Check SSID and password");
            break;
        }
    }
    Serial.println();

    printWifiStatus();

    Serial.println("Connected to wifi");
    Serial.print("Udp server started at port ");
    Serial.println(localPort);
    Udp.begin(localPort);

    if (mdns.begin("c64joy", WiFi.localIP()))
        Serial.println("MDNS responder started");

    for (int i=0; i<TOTAL_PINS; i++)
    {
        pinMode(pinsPort0[i], OUTPUT);
        digitalWrite(pinsPort0[i], LOW);
        pinMode(pinsPort1[i], OUTPUT);
        digitalWrite(pinsPort1[i], LOW);
    }

    // Internal LED in LoLin
    pinMode(D0, OUTPUT);
    digitalWrite(D0, HIGH);  // turn it off
}

void loop()
{
    int noBytes = Udp.parsePacket();
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
        digitalWrite(D0, LOW);

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

        Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
        Udp.write("Answer from ESP8266 ChipID#");
        Udp.print(system_get_chip_id());
        Udp.write("#IP of ESP8266#");
        Udp.println(WiFi.localIP());
        Udp.endPacket();

        // Serial.println(received_command);
        // Serial.println();

        // DEBUG: turn off internal LED
        digitalWrite(D0, HIGH);
    }
}

void printWifiStatus()
{
    // print the SSID of the network you're attached to:
    Serial.print("SSID: ");
    Serial.println(WiFi.SSID());

    // print your WiFi shield's IP address:
    IPAddress ip = WiFi.localIP();
    Serial.print("IP Address: ");
    Serial.println(ip);
}
