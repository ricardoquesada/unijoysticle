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


#define UNIJOYSTICLE_VERSION "v0.4.3"

#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <ESP8266WebServer.h>
#include <ESP8266HTTPClient.h>
#include <ESP8266httpUpdate.h>
#include <ESP8266mDNS.h>
#include <WiFiUDP.h>
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
    ERROR_UDP_FAIL = 4,
};

enum {
    MODE_AP = 0,        // AP, creates the unijoysticle wifi network
    MODE_STA = 1,       // STA, tries to connect to SSID. If it fails, then AP
    MODE_WPS = 2,       // WPS, tries to connect to SSID. If it fails, then WPS, if it fails AP.
};

static const uint8_t DEFAULT_MODE = MODE_AP;
static const uint8_t DEFAULT_INACTIVITY_TIMEOUT = 10;

static const unsigned int localPort = 6464;     // local port to listen for UDP packets
static IPAddress __ipAddress;                   // local IP Address
static uint8_t __mode = DEFAULT_MODE;           // how to connect ?
static uint8_t __inactivityTimeout = DEFAULT_INACTIVITY_TIMEOUT;    // inactivity seconds
static int __lastTimeActivity = 0;              // last time when there was activity
static bool __in_ap_mode = false;               // in AP connection? different than __mode, since
                                                // this is not a "mode" but how the connection was established


static const int INTERNAL_LED = D0; // Amica has two internals LEDs: D0 and D4
static const int __pinsPort0[] = {D0, D1, D2, D3, D4};
static const int __pinsPort1[] = {D5, D6, D7, D8, RX};
static const int TOTAL_PINS = sizeof(__pinsPort0) / sizeof(__pinsPort0[0]);

// how many pushes/presses
static int __joyPushes0[] = {0, 0, 0, 0, 0};
static int __joyPushes1[] = {0, 0, 0, 0, 0};

// how many milliseconds it was used
static int __joyTimeUsed0[] = {0, 0, 0, 0, 0};
static int __joyTimeUsed1[] = {0, 0, 0, 0, 0};

// last time (ms) it was pressed. needed to measure the ms
static int __joyLastTimeUsed0[] = {0, 0, 0, 0, 0};
static int __joyLastTimeUsed1[] = {0, 0, 0, 0, 0};

// last joy state
static uint8_t __lastJoy0 = 0;
static uint8_t __lastJoy1 = 0;

static byte packetBuffer[512];             //buffer to hold incoming and outgoing packets

static WiFiUDP __udp;                           // server for joysticks commands
static ESP8266WebServer __settingsServer(80);   // server for settings


void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(115200);
    Serial.setDebugOutput(true);

    EEPROM.begin(128);

    delay(500);

    __inactivityTimeout = getInactivityTimeout();
    __mode = getMode();
    __in_ap_mode = false;

    Serial.printf("\n*** The UniJoystiCle " UNIJOYSTICLE_VERSION " ***\n");
    Serial.printf("\nMode: %d, inactivity timeout:%d\n", __mode, __inactivityTimeout);

    // setting up Station AP
    setupWiFi();
    delay(500);
    printWifiStatus();

    Serial.printf("Udp server started at port: %d\n", localPort);

    delay(2000);

    if (MDNS.begin("unijoysticle"))
        Serial.println("MDNS responder started");
    else fatalError(ERROR_MDNS_FAIL);

    if (__udp.begin(localPort))
        Serial.println("UDP server started");
    else fatalError(ERROR_UDP_FAIL);

    createWebServer();
    __settingsServer.begin();
    Serial.println("HTTP server started");

    // advertize mDNS service
    MDNS.addService("unijoysticle", "udp", localPort);
    delay(100);
    MDNS.addService("http", "tcp", 80);
    delay(100);

    for (int i=0; i<TOTAL_PINS; i++)
    {
        pinMode(__pinsPort0[i], OUTPUT);
        digitalWrite(__pinsPort0[i], LOW);
        pinMode(__pinsPort1[i], OUTPUT);
        digitalWrite(__pinsPort1[i], LOW);
    }
}

void loop()
{
    loopUDP();
    __settingsServer.handleClient();
}

static void loopUDP()
{
    // this flag is not really needed, but I don't know if digitalWrite()
    // consumes battery... so we just avoid calling it if not needed
    static bool inactivityActivated = false;
    const int now = millis();

    int noBytes = __udp.parsePacket();
    if (noBytes == 0) {
        // after 48 days of use, this might trigger an invalid condition
        if ((__inactivityTimeout > 0) && 
            ((now - __lastTimeActivity) / 1000.0f) >= __inactivityTimeout &&
            !inactivityActivated
            ) {
            for (int i=0; i<TOTAL_PINS; i++)
            {
                digitalWrite(__pinsPort0[i], LOW);
                digitalWrite(__pinsPort1[i], LOW);
            }

            inactivityActivated = true;
        }
        return;
    }
    inactivityActivated = false;

    // FIXME: if a packet is received, then that count as activity
    // even invalid packets... but invalid packets should be ignored
    // but not a big deal really
    __lastTimeActivity = now;

    String received_command = "";

    if (noBytes == 2)
    {
        // Protocol v1
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

        const int* pins;
        int* joyPushes;
        int* joyTimeUsed;
        int* joyLastTimeUsed;
        uint8_t lastJoy;

        __udp.read(packetBuffer,noBytes); // read the packet into the buffer

        if (packetBuffer[0] == 0) {
            pins = __pinsPort0;
            joyPushes = __joyPushes0;
            lastJoy = __lastJoy0;
            joyTimeUsed = __joyTimeUsed0;
            joyLastTimeUsed = __joyLastTimeUsed0;
        } else {
            pins = __pinsPort1;
            joyPushes = __joyPushes1;
            lastJoy = __lastJoy1;
            joyTimeUsed = __joyTimeUsed1;
            joyLastTimeUsed = __joyLastTimeUsed1;
        }

        for (int i=0; i<TOTAL_PINS; i++)
        {
            const uint8_t mask = 1<<i;

            // only if it is different
            if ((lastJoy & mask) != (packetBuffer[1] & mask)) {
                if (packetBuffer[1] & mask) {
                    digitalWrite(pins[i], HIGH);
                    joyPushes[i]++;
                    joyLastTimeUsed[i] = now;
                } else {
                    digitalWrite(pins[i], LOW);
                    joyTimeUsed[i] += (now - joyLastTimeUsed[i]);
                }
            }
        }

        // update last joy
        if (packetBuffer[0] == 0)
            __lastJoy0 = packetBuffer[1];
        else
            __lastJoy1 = packetBuffer[1];
    }
    else if (noBytes == 4)
    {
        // Protocol v2
        // packetBuffer[0] = version
        // packetBuffer[1] = ports enabled
        // packetBuffer[2] = joy1
        // packetBuffer[3] = joy2

        __udp.read(packetBuffer, noBytes); // read the packet into the buffer

        // correct version?
        if (packetBuffer[0] == 2) {
            // joy 1 enabled ?
            if (packetBuffer[1] & 0x1) {
                for (int i=0; i<TOTAL_PINS; i++)
                {
                    const uint8_t mask = 1<<i;
                    // only update if different from last state
                    if ((__lastJoy0 & mask) != (packetBuffer[2] & mask)) {
                        if (packetBuffer[2] & mask) {
                            digitalWrite(__pinsPort0[i], HIGH);
                            __joyPushes0[i]++;
                            __joyLastTimeUsed0[i] = now;
                        } else {
                            digitalWrite(__pinsPort0[i], LOW);
                            __joyTimeUsed0[i] += (now - __joyLastTimeUsed0[i]);
                        }
                    }
                }
                __lastJoy0 = packetBuffer[2];
            }

            // joy 2 enabled ?
            if (packetBuffer[1] & 0x2) {
                for (int i=0; i<TOTAL_PINS; i++)
                {
                    const uint8_t mask = 1<<i;
                    // only update if different from last state
                    if ((__lastJoy1 & mask) != (packetBuffer[3] & mask)) {
                        if (packetBuffer[3] & mask) {
                            digitalWrite(__pinsPort1[i], HIGH);
                            __joyPushes1[i]++;
                            __joyLastTimeUsed1[i] = now;
                        } else {
                            digitalWrite(__pinsPort1[i], LOW);
                            __joyTimeUsed1[i] += (now - __joyLastTimeUsed1[i]);
                        }
                    }
                }
                __lastJoy1 = packetBuffer[3];
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

    // always default to AP if couldn't connect with previous modes
    if (!ok)
        ok = setupAP();
}

static bool setupAP()
{
    delay(100);
    WiFi.mode(WIFI_AP);
    delay(100);

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
//    4: inactivity seconds
//             0 = don't check innactivity
//             any other value = how many seconds should pass before reseting the lines
//  5-7: reserved
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
    for (int i=0; i<3; i++) {
        EEPROM.write(i, __signature[i]);
    }
    // Mode
    EEPROM.write(3, DEFAULT_MODE);
    // Inactivity timeout
    EEPROM.write(4, DEFAULT_INACTIVITY_TIMEOUT);

    // unused
    EEPROM.write(5, 0);
    EEPROM.write(6, 0);
    EEPROM.write(7, 0);

    // SSID name (asciiz)
    EEPROM.write(8, 0);
    // SSDI passwrod (asciiz)
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

// in seconds
static uint8_t getInactivityTimeout()
{
    if (!isValidEEPROM()) {
        setDefaultCredentials();
        return 0;
    }
    return EEPROM.read(4);
}

static void setInactivityTimeout(uint8_t seconds)
{
    if (!isValidEEPROM()) {
        setDefaultCredentials();
    }
    EEPROM.write(4, seconds);
    EEPROM.commit();
}


//
// Settings
//
void createWebServer()
{
    static const char *htmlraw = R"html(<html>
<head><title>UniJoystiCle WiFi setup</title></head>
<body>
<h1>The UniJoystiCle WiFi setup</h1>
<h2>Stats</h2>
<ul>
 <li>Firmware: %s</li>
 <li>IP Address: %d.%d.%d.%d</li>
 <li>SSID: %s</li>
 <li>Chip ID: %d</li>
 <li>Last reset reason: %s</li>
 <li>Joy #1 (ms / # movements):</li>
 <ul>
  <li>Up: %dms / %d</li>
  <li>Down: %dms / %d</li>
  <li>Left: %dms / %d</li>
  <li>Right: %dms / %d</li>
  <li>Fire: %dms / %d</li>
 </ul>
 <li>Joy #2 (ms / # movements):</li>
 <ul>
  <li>Up: %dms / %d</li>
  <li>Down: %dms / %d</li>
  <li>Left: %dms / %d</li>
  <li>Right: %dms / %d</li>
  <li>Fire: %dms / %d</li>
 </ul>
</ul>
<form method='get' action='resetstats'>
  <input type='submit' value='Reset Joy Stats'>
</form>
<h2>Settings</h2>
<h4>Set WiFi mode:</h4>
<form method='get' action='mode'>
 <input type='radio' name='mode' value='0' %s> AP<br>
 <input type='radio' name='mode' value='1' %s> STA<br>
 <input type='radio' name='mode' value='2' %s> STA+WPS<br>
 <input type='submit' value='Submit'>
</form>
<small>Reboot to apply changes</small>

<br>
<p>Mode description:</p>
<ul>
 <li>AP (Access Point mode): creates its own WiFi network. The SSID will start with <i>unijoysticle-</i></li>
 <li>STA (Station mode): Tries to connect to a WiFi network using the specified SSID/password. If it fails, it will go into AP mode</li>
 <li>STA+WPS (Station mode with WPS): Tries to connect to a WiFi network by using <a href='https://en.wikipedia.org/wiki/Wi-Fi_Protected_Setup'>WPS</a>. If it fails it will go into AP mode</li>
</ul>
<h4>Set SSID/Password (to be used when in STA mode):</h4>
<form method='get' action='setting'>
 <label>SSID: </label><input name='ssid' length=32/>
 <label>Password: </label><input name='pass' length=64/>
 <br/>
 <input type='submit' value='Submit'>
</form>
<small>Reboot to apply changes</small>

<h4>Inactivity timeout:</h4>
<form method='get' action='inactivity'>
 <label>Inactivity Timeout:</label><input type="text" name="inactivity" onkeypress='return event.charCode >= 48 && event.charCode <= 57' value="%d">(in seconds, from 0 to 255)</input>
 <br/>
 <input type='submit' value='Submit'>
</form>
After how many seconds of inactivity, should it reset the joysticks. "0" disables this feature. Useful in case one of the joystick lines, unintentinally, is left closed (on).

<h4>Reboot device:</h4>
<form method='get' action='restart'>
 <input type='submit' value='Reboot'>
</form>

<h4>Upgrade firmware:</h4>
<a href="/upgrade">Upgrade page</a>

<hr><p><a href='https://github.com/ricardoquesada/unijoysticle/blob/master/DOCUMENTATION.md'>The UniJoystiCle Documentation</a></p>
</body></html>
)html";

    static const char *htmlredirectok = R"html(<html>
<head>
 <meta http-equiv="refresh" content="1; url=/" />
</head>
<body>Success</body>
</html>
)html";

    static const char *htmlredirecterr = R"html(<html>
<head>
 <meta http-equiv="refresh" content="1; url=/" />
</head>
<body>Error</body>
</html>
)html";

    static const char* htmlupgradeconfirm = R"html(<html>
<h1>Firmware upgrade:</h1>
<p>You are about to upgrade the UniJoystiCle WiFi device's firmware.</p>

<ul>
  <li>Current firmware version: %s</li>
  <li>Latest stable firmware version: <a href="http://ricardoquesada.github.io/unijoysticle/bin/LATEST_VERSION.txt">LATEST_VERSION</a></li>
</ul>

<p>As a backup plan (should the worse happen), please read the following:
<a href="https://github.com/ricardoquesada/unijoysticle/blob/master/DOCUMENTATION.md#installing-the-firmware">Installing the firmware</a>
</p>

<button onclick="location.href = '/upgrade_confirm';">UPGRADE</button>
</body></html>
</html>
)html";

    static const char* htmlupgrading= R"html(<html>
<h2>Upgrading firmware... don't unplug the device.</h2>
<p>Upgrade should finish in less than one minute (depends on your internet connection).</p>
<p>Manually reboot the device after the upgrade is finished.</p>
</html>
)html";

    __settingsServer.on("/", []() {
        char buf[2816];
        const int mode = getMode();
        snprintf(buf, sizeof(buf)-1, htmlraw,
                 UNIJOYSTICLE_VERSION,
                 __ipAddress[0], __ipAddress[1], __ipAddress[2], __ipAddress[3],
                 WiFi.SSID().c_str(),
                 ESP.getChipId(),
                 ESP.getResetReason().c_str(),
                 __joyTimeUsed0[0], __joyPushes0[0],
                 __joyTimeUsed0[1], __joyPushes0[1],
                 __joyTimeUsed0[2], __joyPushes0[2],
                 __joyTimeUsed0[3], __joyPushes0[3],
                 __joyTimeUsed0[4], __joyPushes0[4],
                 __joyTimeUsed1[0], __joyPushes1[0],
                 __joyTimeUsed1[1], __joyPushes1[1],
                 __joyTimeUsed1[2], __joyPushes1[2],
                 __joyTimeUsed1[3], __joyPushes1[3],
                 __joyTimeUsed1[4], __joyPushes1[4],
                 (mode == 0) ? "checked" : "",
                 (mode == 1) ? "checked" : "",
                 (mode == 2) ? "checked" : "",
                 getInactivityTimeout()
                 );
        buf[sizeof(buf)-1] = 0;

        delay(0);
        __settingsServer.send(200, "text/html", buf);
    });

    __settingsServer.on("/setting", []() {
        int statusCode = 404;
        String qsid = __settingsServer.arg("ssid");
        String qpass = __settingsServer.arg("pass");
        String content;
        if (qsid.length() > 0 && qpass.length() > 0) {
            saveCredentials(qsid, qpass);
            content = htmlredirectok;
            statusCode = 200;
        } else {
            content = htmlredirecterr;
            statusCode = 404;
        }
        __settingsServer.send(statusCode, "text/html", content);
    });

    __settingsServer.on("/mode", []() {
        String arg = __settingsServer.arg("mode");
        int mode = arg.toInt();
        setMode(mode);
        __settingsServer.send(200, "text/html", htmlredirectok);
    });

    __settingsServer.on("/inactivity", []() {
        String arg = __settingsServer.arg("inactivity");
        int inactivity = arg.toInt();
        setInactivityTimeout(inactivity);
        __settingsServer.send(200, "text/html", htmlredirectok);
    });

    __settingsServer.on("/restart", []() {
        __settingsServer.send(200, "text/html", htmlredirectok);
        delay(1000);
        ESP.restart();
    });
    __settingsServer.on("/resetstats", []() {
        for (int i=0;i<TOTAL_PINS;++i)
        {
            __joyPushes0[i] = 0;
            __joyTimeUsed0[i] = 0;
            __joyPushes1[i] = 0;
            __joyTimeUsed1[i] = 0;
        }
        __settingsServer.send(200, "text/html", htmlredirectok);
    });

    __settingsServer.on("/upgrade", []() {
        char buf[1024];
        snprintf(buf, sizeof(buf)-1, htmlupgradeconfirm,
                 UNIJOYSTICLE_VERSION);
        __settingsServer.send(200, "text/html", buf);
    });
    __settingsServer.on("/upgrade_confirm", []() {
        __settingsServer.send(200, "text/html", htmlupgrading);
        delay(500);
        t_httpUpdate_return ret = ESPhttpUpdate.update("http://ricardoquesada.github.io/unijoysticle/bin/unijoysticle_firmware.bin");

        switch(ret) {
            case HTTP_UPDATE_FAILED:
                Serial.printf("HTTP_UPDATE_FAILED Error (%d): %s", ESPhttpUpdate.getLastError(), ESPhttpUpdate.getLastErrorString().c_str());
                break;

            case HTTP_UPDATE_NO_UPDATES:
                Serial.println("HTTP_UPDATE_NO_UPDATES");
                break;

            case HTTP_UPDATE_OK:
                Serial.println("HTTP_UPDATE_OK");
                break;
        }
    });
}

