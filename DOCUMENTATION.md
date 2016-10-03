# The UniJoystiCle™ Documentation

The UniJoystiCle is a unicycle simulator for the Commodore 64, and much more!

It consists of two major parts:

* The UniJoystiCle WiFi device
* The UniJoystiCle smartphone application


## The WiFi device

<img class="" src="https://lh3.googleusercontent.com/mFqRvn15aHQGBgtxXAztYggZdZfuW5j25pDKyW7hDaVNp3BLgNDDlKC3449NrG75C80gITnOUsPxNA=w914-h1670-no" width="279" height="153" />

The WiFi device goes attached to the Commodore 64, and is the one that generates the real joystick movements. It does so by opening/closing the joystick lines.

In order to work it needs power. Unfortunately the C64 joystick ports DON'T have enough current to power it. The options to power it are:

* Using the barrel jack:
  * Anything between 5v and 12v
  * With at least 320mA
  * Examples:
    * Getting power from the Datasette port [with a custom cable](http://store.go4retro.com/c2n-power/) (recommended, but might need a [PSU with enough amps](http://personalpages.tds.net/~rcarlsen/custom%20ps.html))
    * Using a regular 9v battery (inefficient but works)
* Using the micro USB socket:
  * It can be connected to a computer
  * or using a wall charger

<img class="" src="https://lh3.googleusercontent.com/5Uu3MAIQ_RIE29ikC09bHBXHAzwis3HkVDjGqDbxMKvbfI3rasiv1oEhlubsxZ-cT4UjkOkFRri1iA=w2228-h1670-no" width="326" height="244" />

### Booting the WiFi device

* DO NOT plug the UniJoystiCle WiFi device to the Commodore 64 yet (if you plug it, you won't break anything, it is that it just might not boot)
* Power the UniJoystiCle WiFi device (see above)
* Wait a few seconds and you should see two LEDs. That means that the device booted, and you can connect to it.
  * If you don't see the two LEDs, please go to [Troubleshooting](#troubleshooting)
* Now, you can plug it to the Commodore 64

By default, the WiFi device will start in Access Point mode, and will create a WiFi network named "unijoysticle-xxyyzz". From your smartphone Settings, connect to it...:

<img src="https://lh3.googleusercontent.com/Kkt_ynCu6Gr-lRwZIv7FD3NPJe9ThwKV7B7o-ehp1mntRX290XKERHdLl_rzuo9orh3iG9IrgRdVJg=w1242-h642-no" width="277" height="143" />

...and then launch the UniJoystiCle smartphone app.

## Smartphone Application

The smartphone application can be download for free from the App Store / Google Play:

<a href="https://itunes.apple.com/us/app/unijoysticle-controller/id1130131741?mt=8"><img src="https://lh3.googleusercontent.com/W88cz-0H1Xet1zHxNqrgjTsFjOMdxut9IwKQzOA0yrgjjGt6eGp2N3rq3AanWojjffyIEfCesYz6O18=w5760-h3600-no" width="162" height="48" /></a><a href='https://play.google.com/store/apps/details?id=moe.retro.unijoysticle&utm_source=global_co&utm_medium=prtnr&utm_content=Mar2515&utm_campaign=PartBadge&pcampaignid=MKT-Other-global-all-co-prtnr-py-PartBadge-Mar2515-1'><img alt='Get it on Google Play' src='https://lh3.googleusercontent.com/nUm_upw_pznWfcD9pp71LPhpwdTMd6L7LVBK2Bw3UoAaiD0AFkTc1P6Gfl1MXiy7mOaApxVLdUMWXA=w564-h168-no' width="162" height="48"/></a>

It sends "joystick commands" to the WiFi device. These commands are regular joystick movements: _Up_, _Down_, _Left_, _Right_ and _Fire_. It can send one or multiple commands at the time.

There are five modes: _UniJoystiCle mode_, _D-pad mode_, _Commando mode_, _Gyruss mode_ and _Linear mode_.

<img style="border: 1px solid #000000;" src="https://lh3.googleusercontent.com/6WxRN1ab1AQC3IOeZ5FS5CqgPPqcXYg4DrP6QrpZ3I70MFmGeA08kEUJIg65USncftVCL6qaxCLXQw=w2208-h660-no" width="477" height="142" />


### UniJoystiCle mode

<img class="" src="https://lh3.googleusercontent.com/9XnsNhBu8Vfee_g93H0e7PVlSniZbJbRqUZ2-8IcU_Malzp46xr53bydN0IudDUljb4MV2hYmumDlQs=w5760-h3600-no" width="290" height="163" />      <img class="" src="https://lh3.googleusercontent.com/7Wye9g3xWeS2Qd6zlaVzwnw8oG6a1PXe8oEhlaTTSqte0MqDaqeK3QPwDL0CMMKH_ahilKYZGOoqmPw=w5760-h3600-no" width="87" height="155" />

In this mode, the joystick commands are generated from the smartphone's accelerometer. The accelerometer detects acceleration in the three coordinates: X, Y and Z. If we attach an accelerometer (in this case the smartphone) to a unicycle (or bike) pedal, and we measure the accelerometer's data while we pedal, we will notice that the X and Z axis generates data similar to a sine and cosine.

<img src="https://lh3.googleusercontent.com/tkTDxICQD2L51ClgOARPCSrcaHvqfLueInfqmEwKdEmb66_2tL7SaZwHTRsgiKj-FA-gGCwA77vmw-4=w5760-h3600-no" />

And if we put those values in cartesian coordinates (eg: Accelerometer(Z) = Y axis, and Accelerometer(X) = X axis), we will have something similar to a circle.

<img src="https://lh3.googleusercontent.com/uj7t3w1rtLdae9kZk7fIKbxeKU4jssFHRcn8Q4HucrirkWqimleXumDhUDdBKWA74yHrAK-drD1CV6Y=w5760-h3600-no" width="238" height="212" />

And if we calculate the arctangent (`atan2()`) value of sin/cosine we get the angle. And with the angle we can simulate the joystick movements. We divide the circle in 8 segments, and assign a joystick movement to each segment. Eg:

<img src="https://lh3.googleusercontent.com/JWfiNmE3_R_m17V_DKnyVQjtzHvR8iYC3SUTGfIaZl4M3q7MUluOVOKcKI9tCLDD47IhvnoRSsHw6-Y=w5760-h3600-no" width="224" height="224" />

There are no such things as "joystick diagonal commands". So in order to simulate the top-right diagonal, we just send the "top" and "right" joystick movements at the same time.

From the graphic we can deduce that:
<p style="padding-left: 30px;"><strong>One pedal revolution == one joystick "revolution"</strong></p>
But it is possible to change that ratio. For example, we can change it to "One pedal revolution == <strong>two</strong> joystick revolutions". Just go to settings and do:

<img class="" src="https://lh3.googleusercontent.com/lzJpTVJDyCto_gNurvpjZwImO1-O9_az0cfN0KCDsijJ0y2oPRPJCk7QZlEXUj3jlmRiQFtNZFll3Ig=w5760-h3600-no" width="376" height="172" />

A ratio of 2.0 will divide the circle in 16 segments (8 segments * 2.0 ratio == 16 segments), and what will happen is that:
<p style="padding-left: 30px;">One pedal revolution == Two joystick revolutions</p>


<img src="https://lh3.googleusercontent.com/PfU204uuYq_qwNGXRHQZ3aRuhFpp6ZkmERjB8gwByobmwdCu0TR7QMp-SbN2n_t44PwkGGrMPZYBwqk=w5760-h3600-no" width="224" height="224" />

This is useful to:

* Increase your speed when you have an slow unicycle (eg: long cranks or big wheel size), or you don't pedal fast enough
* You want to play face-to-face with another rider, but one has an slower unicycle

__Firing:__ In order to _fire_, you have to hop. Settings has a "Jump Threshold" settings that is used to control how hard you have to hop.

__Compatibility:__ This mode is compatible with multiple sports games like:

* [The Uni Games](https://github.com/ricardoquesada/c64-the-uni-games) (in fact this mode was created to play this game)
* Sports games like [Hypersports](http://gamebase64.com/game.php?id=3666&d=18&h=0), [Decathon](http://gamebase64.com/game.php?id=2057&d=18&h=0), etc.
* [C64anabalt](http://www.rgcd.co.uk/2011/12/c64anabalt-c64-2011.html), and other jump-only-games
* and more

__Video:__ [Using a unicycle to play The Uni Games ](https://www.youtube.com/watch?v=w2cdoTU-GJU)

__Video:__ [Using one foot to play C64anabalt](https://www.youtube.com/watch?v=n7p1p53uisk)

### D-pad mode

<img class="" src="https://lh3.googleusercontent.com/j2hgT6dzu-wSK1eEwbvD97THXVmokBJV3RkF1lzUeKNXd4A9eqhKjyRG_5rlkAtHfWi4_T6sw1AFdB8=w5760-h3600-no" width="343" height="193" />

This mode converts the smartphone in a virtual D-pad. Just press the arrows for joysticks movements, and the circle to fire.

__Compatibility:__

* Any game can be played in this mode.

__Using this mode:__ [YouTube video](https://youtu.be/RhEGogreM0E)


### D-pad + Game Controllers

<img class="alignnone" src="https://lh3.googleusercontent.com/WRLXh522eTsfwQogNMmFebIEC4v-6AjRLbbWMC2ox5t7a4yezb_PzxK8bZKYrT8xIuVOplTMHRmSn00=w5760-h3600-no" width="304" height="171" /><img class="alignnone" src="https://lh3.googleusercontent.com/vxbiIsJjtjfyqieu4QO27VW6hBTxszUth8Eb_g6TbgVK1jGEmvNmnazd7CAb1uFo6I38RNpK_qZO0sc=w5760-h3600-no" width="273" height="157" />

This is the same D-pad mode as before. But if you <a href="http://www.howtogeek.com/242223/how-to-use-a-physical-game-controller-with-an-iphone-ipad-or-android-device/">connect any Game Controller to your smartphone</a> (for iOS it supports both <a href="https://afterpad.com/mficontrollers/">MFi</a> and <a href="https://www.ionaudio.com/products/details/icade">iCade</a> controllers; for Android it supports official controllers as well as OUYA ones), it will automatically detect it. And with it you can control the virtual D-pad giving you a great gaming experience.

You can configure it to:

* Use Button "B" to jump. When enabled, it will disable to "Up" D-pad arrow
* Swap Buttons "A" and "B" (swaps Jump and Shoot buttons)

<img class="" src="https://lh3.googleusercontent.com/-RH__wcCr293gHL_KbyM4Nk61MMs1qjJgMI4GHXK1iFUM0Q7VZY3cM1Vt0Pm6dwaVe2tKp6J5Uf0swk=w5760-h3600-no" width="282" height="95" />

__Compatibility:__

* Any game can be played in this mode.
* Optimized for platformer games like [Giana Sisters](http://gamebase64.com/game.php?id=3275&amp;d=18&amp;h=0), [Super Bread Box](http://gamebase64.com/game.php?id=24140&amp;d=18&amp;h=0), [Bubble Bobble](http://gamebase64.com/game.php?id=1138&amp;d=18&amp;h=0), etc.

__Using this mode:__ [YouTube video](https://www.youtube.com/watch?v=0cdgaYVYaao)

__Which controller should you use:__

For iOS:
* For MFi controllers, try here: [AfterPad](https://afterpad.com/mficontrollers/">https://afterpad.com/mficontrollers/). The [Nimbus SteelSeries](https://afterpad.com/steelseries-nimbus-the-afterpad-review/) is good. DO NOT get the Status SteelSeries.
* For iCade controlles, try here: [iCade Controllers](http://retrorgb.com/icadecontrollers.html). The ["original" iCade](http://retrorgb.com/icade.html) is good.

For Android:
* Any Game Controller that supports __the Android protocol__. See [reviews here](http://www.androidauthority.com/best-bluetooth-gaming-controllers-403184/) [and here](https://www.slant.co/topics/2074/~third-party-game-controllers-for-android)
* OUYA Game Controllers (they are not that great though). [Get it here](http://www.ebay.com/sch/i.html?_from=R40&_trksid=p2050601.m570.l1313.TR0.TRC0.H0.Xouya+game+controller.TRS0&_nkw=ouya+game+controller&_sacat=0)
* Notes:
  * Amazon Fire Game Controllers support the Android protocol
  * Moga Game Controllers support the Android protocol [when in "B" mode](https://gavinsgadgets.com/2016/03/21/how-to-connect-a-moga-pro-power-bluetooth-game-controller-with-the-samsung-galaxy-s7/)
  * WiFi-Direct game controllers are supported as well, like the NVIDIA SHIELD, or the Amazon Fire 2nd gen. But since most smartphones don't support WiFi-Direct, you should install the Android UniJoystiCle app into WiFi-Direct-enabled devices, like the Amazon Fire TV, or the NVIDIA SHIELD Android TV.

### Commando mode + Game Controllers

<img src="https://lh3.googleusercontent.com/tgLsbc3oRG6yk8MqKIIltiFPOOrm23AVACod9iCpep07wRRlDrkCePVUI-9nbm7v8UcY10qXQhP-Mw=w1136-h638-no" width="344" height="194" /><img class="alignnone" src="https://lh3.googleusercontent.com/vxbiIsJjtjfyqieu4QO27VW6hBTxszUth8Eb_g6TbgVK1jGEmvNmnazd7CAb1uFo6I38RNpK_qZO0sc=w5760-h3600-no" width="273" height="157" />


In this mode you control both joysticks at the same time. A game controller is needed for this. Can't be played with the "virtual" dpad.
It is called "Commando" mode, since it allows you to play the Commando game by using:

* The dpad to control the movements of the Hero
* Button A to fire
* and Button B to throw grenades. Spacebar is not needed anymore!

You can do much more than that, since you control both joysticks. So you can play multiplayer games controlling both players at the same time. Not for the mere mortals.

__Compatibility:__

* Any game that uses Joystick #2 + spacebar can be used in this mode.
* Any dual player game can be played in this mode.

__Using this mode:__ [YouTube video](https://www.youtube.com/watch?v=VK-gzT5wkiw)

__Which controller should you use:__

Refer to _D-pad + Game Controllers_ section.


### Gyruss mode

<img src="https://lh3.googleusercontent.com/E17hGhA4Ab6wNy1_OnpuzIwfzb3y3nsBFTVZv70gY1UJGSlOiXEGEOxda_YRd43CVQuFlhRbvvbWwIM=w5760-h3600-no" width="344" height="194" />

In this mode you control the black circle that is inside the blue circle strip. You move the black ball by tilting your smartphone, and gravity will do the rest. Press the gray circle at the right for _fire_.

<img class="" src="https://lh3.googleusercontent.com/_mIk_b2YmrhLqCo0IGk81Euw4JsGlK1siu131e67ypI4aCKCxBo59Xu37o-P2QZzeVm3JL1aCFVs928=w5760-h3600-no" width="304" height="59" />

__Compatibility:__

* [Gyruss](http://gamebase64.com/game.php?id=3370&d=18&h=0)
* all the <em>UniJoystiCle mode</em> games
* and games with similar techniques to Gyruss

__Using this mode:__ [YouTube video](https://youtu.be/n2YHoj1pXB8)

__Note__: This mode is only avaible on iOS. Android support coming soon.

### Linear mode

<img class="" src="https://lh3.googleusercontent.com/TcfUvufZWZBwNOpHt9y6_y6MEsXQ90xKGHmvJQAzr0p7kCwuvAfgm4LqLzYp0jDJMphJHG5LhnZQ25k=w5760-h3600-no" width="344" height="194" />

This mode is not meant to play games. Instead, is meant to convert the joysticks signals into a numbers from 0 to 31. Since each joystick has 5 lines (5 bits), we have 5 ^^ 2 == 32 combinations.

__Compatibility:__

* [C64 I.D.IoT.R](https://github.com/ricardoquesada/c64-idiotr), the Intelligent Dimmer for IoT RRRRR

__Using this mode:__ [YouTube video](https://www.youtube.com/watch?v=eKlaUfoTuYQ)

## Installing the firmware

* Make sure that the UniJoystiCle Wifi device is NOT plugged into the C64
* Connect the UniJoystiCle Wifi device to the computer using the micro USB cable
* Install the [CP2104 Serial-to-USB driver](https://www.silabs.com/products/mcu/Pages/USBtoUARTBridgeVCPDrivers.aspx)

And from here you have two options: the "easy" one for users, and the "hard" one for developers:

### Option A: For users only

Either you can use:
* [esptool.py](https://github.com/themadinventor/esptool) (a CLI tool for Windows, Linux and Mac)
* or [NodeMCU flasher](https://github.com/nodemcu/nodemcu-flasher) a GUI tool (Windows only)

For `esptool.py` you should do:

* Download the latest UniJoystiCle firmware from here: [unijoysticle_firmware.bin](http://ricardoquesada.github.io/unijoysticle/bin/unijoysticle_firmware.bin)
* Install [esptool](https://github.com/themadinventor/esptool)
  * `$ pip install esptool`
* Execute: `$ sudo python esptool.py --port /dev/cu.SLAB_USBtoUART write_flash -fm dio -fs 32m 0x00000 /path/to/unijoysticle_firmware.bin`

In case it is needed, it uses a:
  * ESP8266-12e module
  * It has a 4Mbyte (32Mbit) flash
  * use 0x00000000 as offset

For detailed info, read: [Flashing the NodeMCU firmware](https://nodemcu.readthedocs.io/en/dev/en/flash/)

Since firmware v0.4.2, manual upgrading no longer needed to. It has an option to upgrade itself.

### Option B: For developers only
* Install [PlatformIO](http://platformio.org)
* Clone the [UniJoystiCle github repo]("https://github.com/ricardoquesada/unijoysticle)
* And do `make && make upload`

Example:
```
$ git clone https://github.com/ricardoquesada/unijoysticle.git
$ cd unijoysticle/esp8266_firmware/firmware
$ make
$ make upload
```

## Assembling the PCB

The hardware as well as the software are open source. So you can build one yourself. The schematic and board files are in [Eagle](https://cadsoft.io/) format:

* board: <a href="https://github.com/ricardoquesada/unijoysticle/blob/unijoysticle-v0.3/schematic/unijoysticle.brd">unijoysticle.brd</a> (v0.2.2)
* schematic: <a href="https://github.com/ricardoquesada/unijoysticle/blob/unijoysticle-v0.3/schematic/unijoysticle.sch">unijoysticle.sch</a> (v0.2.2)

You will need the following components (BOM):

* 1 x [NodeMCU ESP8266](http://www.aliexpress.com/item/New-Wireless-module-NodeMcu-Lua-WIFI-Internet-of-Things-development-board-based-ESP8266-with-pcb-Antenna/32303690854.html)
* 2 x [DB9 female](https://www.digikey.com/product-detail/en/assmann-wsw-components/A-DF-09-A-KG-T2S/AE10921-ND/1241800)
* 3 x [4066 IC](https://www.digikey.com/product-detail/en/texas-instruments/SN74HC4066N/296-8329-5-ND/376726)
* 3 x [sockets for the ICs](https://www.digikey.com/product-detail/en/on-shore-technology-inc/SA143000/ED3014-ND/3313545)
* 1 x [barrel jack](https://www.digikey.com/product-detail/en/cui-inc/PJ-002A/CP-002A-ND/96962)
* 1 x [4001 diode](https://www.digikey.com/product-detail/en/micro-commercial-co/1N4001-TP/1N4001-TPMSCT-ND/773688)
* [Pins](http://www.aliexpress.com/item/Free-shppping-3200pin-1x40-Pin-2-54mm-Single-Row-Female-Pin-Header-Connector-80PCS-LOT/1897568598.html)

And solder them. It should be straightforward where to place the components. If not, [ping me](https://twitter.com/ricardoquesada)

The barrel jack and the diode are optional. They are needed if you want to power the NodeMCU module with an external power source. eg: from the C64 datasette port, or from a 9v battery. Any DC power from 5v to 12v should be good. 320mA should be enough.
If you want to avoid the soldering, you can __[order an already assembled PCB from here](https://retro.moe/unijoysticle/)__.

After assembling the PCB, it should look like this:

<img class="" src="https://lh3.googleusercontent.com/yPbYMpfy4th501PObRIPJa0vGIGubWI4YZfMf8zcqMMhFt0tN0teuPUGoLKu244fXXxJ_c4n95gNrQ=w2464-h1670-no" width="468" height="318" />

<img src="https://lh3.googleusercontent.com/EzcB9pZfIeSFlQqB6ONgqUOGPxgydRiCND_uexf3BiooADe20sfCTA-bgKs2NJLEUY6ANjG9HFU8dA=w2528-h1586-no" width="482" height="302" />


## Troubleshooting

The first thing to do: download and run this C64 program. Then follow its intructions:

  * [UniJoystiCle test program](https://github.com/ricardoquesada/unijoysticle/raw/master/tests/bin/unijoysticle_test_exo.prg)

<img src="https://lh3.googleusercontent.com/35TVPVQQHc_SImpSP1NEPhm6N-kkvyMSqXgO9m3gqSQUQoNxWX5dR6NjFWgl7NJoDzhtPdLpI9-djA=w384-h272-no" />

### The device (NodeMCU) doesn't boot

If you don't see two LEDs on the NodeMCU then:

* Make sure it is NOT plugged into the Commmodore 64
* Power it either:
  * via the micro USB cable
  * or by using the barrel jack: anything between 5V and 12V with __at least 320mA__ should be Ok.
* Press the RST (reset) button on the NodeMCU

If that doesn't work (you don't see the two LEDs), probably:

* try booting it again. Just press the `RST` button that is in the NodeMCU
* or perhaps your NodeMCU doesn't have the UniJoystiCle firmware. Go to the "Installing the firmware" section
* you are using power from c64 (datasette, user port or expansion port), and the C64 doesn't have enough current. See [When using power from the C64](#when-using-power-from-the-c64))

#### When using power from the C64

You should know that all the connected devices compete for the current. Example: If you have an 1541 Ultimate II, a WiModem and the UniJoystiCle, then the three of them will compete for current. If that is the case, most probably one of them won't work.

Tested configurations with __current taken from the datasette port__:

* UniJoystiCle + regular c64 PSU + C64: Works Ok.
* UniJoystiCle + 1541 Ultimate II + regular c64 PSU + c64: Works Ok.
* UniJoystiCle + Turbo Chameleon 64 + regular c64 PSU + c64: Sometimes it works, sometimes it doesn't
* UniJoystiCle + Turbo Chameleon 64 + [Ray Carlen's PSU](http://personalpages.tds.net/~rcarlsen/custom%20ps.html) + c64: Works Ok
* UniJoystiCle + Turbo Chameleon 64 + WiModem + [Ray Carlen's PSU](http://personalpages.tds.net/~rcarlsen/custom%20ps.html) + c64: Works Ok
* UniJoystiCle + C128D: Works Ok

#### So, How much power does The UniJoystiCle use ?

The WiFi module alone uses up to 200mA when it is in Access Point and the clients try to connect to it. But on average it uses ~56mA. More info here: [ESP8266 Power Consumption](http://bbs.espressif.com/viewtopic.php?t=133).
It also uses 12mA for each joystick line that is High. For example, if 3 lines are High (Joy #1 Up, Joy #1 Fire, joy #Left) then it will use 36mA for that alone.
So, a max of 320mA might be needed:

* Up to 200mA for the WiFi module (but the average ~56mA)
* plus 120mA when all ten joystick lines are High


### Joysticks don't work

* From your smartphone, make sure that you are connected to the Unijoysticle WiFi network
  * If it fails to connect, try again. Sometimes it take a few tries to connect
  * No more than 2 connections will be accepted (at least in firmware 0.3.1)
* From the smartphone UniJoystiCle application, make sure that the WiFi device address is:
  * `192.168.4.1` (when in Access Point mode)
  * or `unijoysticle.local` (which will work both for Access Point and Station modes)

### The C64 keyboard doesn't work

This is because the UniJoystiCle smartphone application is sending joystick commands to the C64.
Just make sure that the UniJoystiCle smartphone application is not running.

This is not a bug/limitation of the UniJoystiCle. It is a c64 limitation.
The c64 keyboard won't work correctly if you keep moving a regular joystick while using the keyboard.

Since firmware v0.4.3, a default inactivity timeout of 10 seconds was added.
If no data is received within that period of time, the joysticks are going to be reset.


### The game controllers don't work

* For iOS, all MFi (the official) and all iCade (the "old" unofficial) game controllers should work.
* For Android, all the official (including Amazon, Nvidia, Moga game controllers) and the OUYA game controllers should work.
* Make sure that the Game Controller is paired via Bluetooth to the smartphone
  * If unsure, [read this guide](http://www.howtogeek.com/242223/how-to-use-a-physical-game-controller-with-an-iphone-ipad-or-android-device/)
* When the game controller is paired to the smartphone, you can use it in the D-pad or Commando modes:
  * on iOS you should see a legend that says "Game controller detected" (MFi controllers). Or nothing if using iCade controllers.
  * on Android you should see the name of the game controller in the title


### Some joystick movements work but others don't

* Probably one (or more) of the 4066 ICs are malfunctioning. Try swapping them until you find the broken one. And then replace it with a new one
* or perhaps one of the traces in the PCB is open. With a multimeter and the schematic you should be able to find it.


## Bugs

Did you find a bug? Please report it here:

* [UniJoystiCle Bugs](https://github.com/ricardoquesada/unijoysticle/issues)
