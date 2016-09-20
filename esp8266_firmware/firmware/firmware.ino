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

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <WiFiUDP.h>
#include <ESP8266mDNS.h>
#include <EEPROM.h>

extern "C" {
  #include "user_interface.h"
}

static const char* __ssid_ap = "unijoysticle";       // SSID for Access Point
static const char __signature[] = "uni";

enum {
    // possible errors. Use nubmer >=2
    ERROR_CANNOT_CONNECT = 2,
    ERROR_MDNS_FAIL = 3,
};

enum {
    MODE_AP = 0,        // AP, creates the unijoysticle wifi network
    MODE_STA = 1,       // STA, tries to connect to SSID. If it fails, then AP
    MODE_WPS = 2,       // WPS, tries to connect to SSID. If it fails, then WPS, if it fails AP.
};

static const unsigned int localPort = 6464;     // local port to listen for UDP packets
static IPAddress __ipAddress;                   // local IP Address
static uint8_t __mode = MODE_AP;                // how to connect ?
static bool __in_ap_mode = false;               // in AP connection? different than __mode, since
                                                // this is not a "mode" but how the connection was established


static const int INTERNAL_LED = D0; // Amica has two internals LEDs: D0 and D4
static const int pinsPort0[] = {D0, D1, D2, D3, D4};
static const int pinsPort1[] = {D5, D6, D7, D8, RX};
static const int TOTAL_PINS = sizeof(pinsPort0) / sizeof(pinsPort0[0]);

static byte packetBuffer[512];             //buffer to hold incoming and outgoing packets

static WiFiUDP __udp;                           // server for joysticks commands
static MDNSResponder __mdns;                    // announce Joystick service
static ESP8266WebServer __settingsServer(80);   // server for settings


void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    EEPROM.begin(128);

    delay(500);

    __mode = getMode();
    __in_ap_mode = false;

    Serial.printf("\n*** The UniJoystiCle " UNIJOYSTICLE_VERSION " ***\n");
    Serial.printf("\nMode: %d\n", __mode);

    // setting up Station AP
    setupWiFi();
    delay(500);
    printWifiStatus();

    Serial.printf("Udp server started at port: %d\n", localPort);

    delay(2000);

    __udp.begin(localPort);

    createWebServer();
    __settingsServer.begin();

    if (__mdns.begin("unijoysticle", __ipAddress)) {
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
    __mdns.addService("unijoysticle", "udp", localPort);
    __mdns.addService("http", "tcp", 80);

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
    __settingsServer.handleClient();
    loopUDP();
}

static void loopUDP()
{
    int noBytes = __udp.parsePacket();
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
        // Serial.print(__udp.remoteIP());
        // Serial.print(":");
        // Serial.println(__udp.remotePort());
        // We've received a packet, read the data from it
        __udp.read(packetBuffer,noBytes); // read the packet into the buffer

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

        // __udp.beginPacket(__udp.remoteIP(), __udp.remotePort());
        // __udp.write("#IP of ESP8266#");
        // __udp.println(WiFi.localIP());
        // __udp.endPacket();

        // Serial.println(received_command);
        // Serial.println();

        // DEBUG: turn off internal LED
        // digitalWrite(INTERNAL_LED, HIGH);
    }
    else if (noBytes == 4)
    {
        __udp.read(packetBuffer,noBytes); // read the packet into the buffer

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
    bool ok = false;

    if (__mode == MODE_STA || __mode == MODE_WPS)
        ok = setupSTA();

    if (!ok && __mode == MODE_WPS)
        ok = setupWPS();

    // always defualt in AP if couldn't connect with previous modes
    if (!ok)
        ok = setupAP();
}

static bool setupAP()
{
    delay(500);
    WiFi.mode(WIFI_AP);
    delay(500);

    uint8_t mac[WL_MAC_ADDR_LENGTH];
    WiFi.softAPmacAddress(mac);
    char buf[50];
    memset(buf, 0, sizeof(buf)-1);
    snprintf(buf, sizeof(buf)-1, "%s-%x%x%x",
             __ssid_ap,
             mac[WL_MAC_ADDR_LENGTH-3],
             mac[WL_MAC_ADDR_LENGTH-2],
             mac[WL_MAC_ADDR_LENGTH-1]);

    Serial.printf("Creating AP with SSID='%s'...",buf);
    bool success = false;
    while(!success) {
        if ((success=WiFi.softAP(buf))) {
            Serial.println("OK");
        } else {
            Serial.println("Error");
            delay(1000);
        }
    }

    __in_ap_mode = true;
    return true;
}

static bool setupSTA()
{
    char ssid[128];
    char pass[128];
    readCredentials(ssid, pass);

    Serial.printf("Trying to connect to %s...\n", ssid);
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, pass);
    return (WiFi.waitForConnectResult() == WL_CONNECTED);
}

static bool setupWPS()
{
    // Mode must be WIFI_STA, but it is already in that mode
    Serial.println("Trying to connect using WPS...");
    bool wpsSuccess = WiFi.beginWPSConfig();
    if(wpsSuccess) {
        // in case of a timeout we might have an empty ssid
        String newSSID = WiFi.SSID();
        if(newSSID.length() > 0) {
            Serial.printf("Connected to SSID '%s'\n", newSSID.c_str());
            Serial.printf("Password: %s\n", WiFi.psk().c_str());
            saveCredentials(WiFi.SSID(), WiFi.psk());
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
    return wpsSuccess;
}

static void printWifiStatus()
{
    if (__in_ap_mode) {
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
        Serial.printf("SSID: %s\n", WiFi.SSID().c_str());
        // print your WiFi shield's IP address:
        __ipAddress = WiFi.localIP();
        Serial.print("Local IP Address: ");
        Serial.println(__ipAddress);
    }
}

//
// EEPROM struct
//  0-2: "uni"
//    3: mode: 0 = AP, creates the unijoysticle wifi network
//             1 = STA, tries to connect to SSID. If it fails, then AP
//             2 = WPS, tries to connect to SSID. If it fails, then WPS, if it fails AP.
//  4-7: reserved
//  asciiz: SSID
//  asciiz: password
//
static void readCredentials(char* ssid, char* pass)
{

#if 0
    char tmp[513];
    for (int i=0; i<512; i++)
        tmp[i] = EEPROM.read(i);
    Serial.printf("EEPROM signature: %s\n", tmp);

    for (int i=0;i<32;i++) {
        Serial.printf("%2x %2x %2x %2x %2x %2x %2x %2x - "
                      ,tmp[i*16 + 0]
                      ,tmp[i*16 + 1]
                      ,tmp[i*16 + 2]
                      ,tmp[i*16 + 3]
                      ,tmp[i*16 + 4]
                      ,tmp[i*16 + 5]
                      ,tmp[i*16 + 6]
                      ,tmp[i*16 + 7]);
        Serial.printf("%2x %2x %2x %2x %2x %2x %2x %2x\n"
                      ,tmp[i*16 + 8]
                      ,tmp[i*16 + 9]
                      ,tmp[i*16 + 10]
                      ,tmp[i*16 + 11]
                      ,tmp[i*16 + 12]
                      ,tmp[i*16 + 13]
                      ,tmp[i*16 + 14]
                      ,tmp[i*16 + 15]);
    }
#endif

    if (!isValidEEPROM()) {
        Serial.printf("EEPROM signature failed: not 'uni'\n");
        ssid[0] = 0;
        pass[0] = 0;
        setDefaultCredentials();
        return;
    }

    int idx=8;
    for(int i=0;;i++) {
        ssid[i] = EEPROM.read(idx++);
        if (ssid[i] == 0)
            break;
    }

    for(int i=0;;i++) {
        pass[i] = EEPROM.read(idx++);
        if (pass[i] == 0)
            break;
    }
    Serial.printf("EEPROM credentials: ssid: %s, pass: %s\n", ssid, pass);
}

static void saveCredentials(const String& ssid, const String& pass)
{
    int idx = 8;

    for (int i=0; i<ssid.length(); ++i) {
        EEPROM.write(idx, ssid[i]);
        idx++;
    }

    EEPROM.write(idx, 0);
    idx++;

    for (int i=0; i<pass.length(); ++i) {
        EEPROM.write(idx, pass[i]);
        idx++;
    }
    EEPROM.write(idx, 0);
    EEPROM.commit();
}

static bool isValidEEPROM()
{
    bool failed = false;
    for (int i=0; i<3; ++i) {
        char c = EEPROM.read(i);
        failed |= (c != __signature[i]);
    }
    return !failed;
}

static void setDefaultCredentials()
{
    for (int i=0; i<8; i++) {
        if (i<3)
            EEPROM.write(i, __signature[i]);
        else
            EEPROM.write(i, 8);
    }
    EEPROM.write(8, 0);
    EEPROM.write(9, 0);
    EEPROM.commit();
}

static uint8_t getMode()
{
    if (!isValidEEPROM()) {
        setDefaultCredentials();
        return MODE_AP;
    }
    uint8_t mode = EEPROM.read(3);
    return mode;
}

static void setMode(uint8_t mode)
{
    if (!isValidEEPROM()) {
        setDefaultCredentials();
    }
    EEPROM.write(3, mode);
    EEPROM.commit();
}


//
// Settings
//
void createWebServer()
{
    __settingsServer.on("/", []() {
        String content = "<!DOCTYPE HTML>\r\n<html><head><title>The UniJoystiCle</title></head><body><h1>The UniJoystiCle</h1>";
        content += "<h2>Stats</h2>" \
                "<p>Firmware: " UNIJOYSTICLE_VERSION"</p>" \
                "<p>IP Address: ";
        content += String(__ipAddress[0]) + "." + String(__ipAddress[1]) + "." + String(__ipAddress[2]) + "." + String(__ipAddress[3]);
        content += "</p>";

        //
        // Radio Buttons
        //
        int mode = getMode();
        String stringMode = "AP";
        if (mode == 1)
            stringMode = "STA";
        else if (mode == 2)
            stringMode = "STA+WPS";
        content += "<h2>Settings</h2>" \
                "<p>Change WiFi mode (current mode: ";
        content += stringMode;
        content += ")</p><form method='get' action='mode'>" \
                    "<input type='radio' name='mode' value='0'";
        if (mode==0)
            content += "checked";
        content += "> AP<br>";

        content += "<input type='radio' name='mode' value='1'";
        if (mode==1)
            content += "checked";
        content += "> STA<br>";

        content += "<input type='radio' name='mode' value='2'";
        if (mode==2)
            content += "checked";
        content += "> STA+WPS<br>" \
                    "<input type='submit' value='Submit'></form>";

        //
        // Change SSID/password
        //
        content += "<p>Change SSID/Password (to use when in STA mode):</p>" \
                    "<form method='get' action='setting'><label>SSID: </label><input name='ssid' length=32><input name='pass' length=64><input type='submit' value='Submit'></form>";

        //
        // Reset
        //
        content += "<p>Reset device:</p><form method='get' action='reset'>" \
                    "<input type='submit' value='Reboot'></form>";

        //
        // Definition
        //
        content += "<br>";
        content += "<p>WiFi Mode:</p>";
        content += "<li>AP (Access Point mode): creates its own WiFi network. The SSID will start with 'unijoysticle-'</li>";
        content += "<li>STA (Station mode): Tries to connect to a WiFi network using the specified SSID/password. If it fails, it will go into AP mode</li>";
        content += "<li>STA+WPS (Station mode with WPS): Tries to connect to a WiFi network by using using <a href='https://en.wikipedia.org/wiki/Wi-Fi_Protected_Setup'>WPS</a>. If it fails it will go into AP mode</li>";
        content += "</ul>";

        content += "<hr><p><a href='http://retro.moe/unijoysticle'>The UniJoystiCle homepage</a></p>";

        delay(50);

        content += "</body></html>";
        __settingsServer.send(200, "text/html", content);
    });

    __settingsServer.on("/setting", []() {
        int statusCode = 404;
        String qsid = __settingsServer.arg("ssid");
        String qpass = __settingsServer.arg("pass");
        String content;
        if (qsid.length() > 0 && qpass.length() > 0) {
            saveCredentials(qsid, qpass);
            content = "{\"Success\":\"saved to EEPROM... reset to boot into new WiFi\"}";
            statusCode = 200;
        } else {
            content = "{\"Error\":\"404 not found\"}";
            statusCode = 404;
            Serial.println("Sending 404");
        }
        __settingsServer.send(statusCode, "application/json", content);
    });

    __settingsServer.on("/mode", []() {
        String arg = __settingsServer.arg("mode");
        int mode = arg.toInt();
        setMode(mode);
        String content = "{\"Success\":\"saved to EEPROM... reset to boot into new WiFi\"}";
        __settingsServer.send(200, "application/json", content);
    });

    __settingsServer.on("/reset", []() {
        String content = "{\"Success\":\"Rebooting...\"}";
        __settingsServer.send(200, "application/json", content);
        delay(500);
        ESP.restart();
    });
}

