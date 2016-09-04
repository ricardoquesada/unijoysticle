# The UniJoystiCle Documentation

## Smartphone Application

The smartphone application can be download from free from the AppStore, or you can download the source code from [github](https://github.com/ricardoquesada/unijoysticle):

<a href="https://itunes.apple.com/us/app/unijoysticle-controller/id1130131741?mt=8"><img class="alignnone" src="https://lh3.googleusercontent.com/W88cz-0H1Xet1zHxNqrgjTsFjOMdxut9IwKQzOA0yrgjjGt6eGp2N3rq3AanWojjffyIEfCesYz6O18=w5760-h3600-no" width="162" height="48" /></a><a href='https://play.google.com/store/apps/details?id=moe.retro.unijoysticle&utm_source=global_co&utm_medium=prtnr&utm_content=Mar2515&utm_campaign=PartBadge&pcampaignid=MKT-Other-global-all-co-prtnr-py-PartBadge-Mar2515-1'><img alt='Get it on Google Play' src='https://play.google.com/intl/en_us/badges/images/generic/en_badge_web_generic.png' width="180" height="70"/></a>

It sends "joystick commands" to the UniJoystiCle WiFi receiver. These commands are regular joystick movements: _Up_, _Down_, _Left_, _Right_ and _Fire_. It can send one or multiple commands at the time.

### Setup

* DO NOT plug the UniJoystiCle WiFi receiver to the Commodore 64 yet
* Power the UniJoystiCle WiFi receiver (either with a micro USB cable, or via the barrel jack)
* Wait a few seconds and you should see two LEDs. The color is not important

<img class="" src="https://lh3.googleusercontent.com/mFqRvn15aHQGBgtxXAztYggZdZfuW5j25pDKyW7hDaVNp3BLgNDDlKC3449NrG75C80gITnOUsPxNA=w914-h1670-no" width="279" height="153" />

After a booting it, you should be able to see a WiFi network called "unijoysticle-xxyy". From your smartphone Settings, connect to it...:

<img src="https://lh3.googleusercontent.com/Kkt_ynCu6Gr-lRwZIv7FD3NPJe9ThwKV7B7o-ehp1mntRX290XKERHdLl_rzuo9orh3iG9IrgRdVJg=w1242-h642-no" width="277" height="143" />

...and then launch the UniJoystiCle Controller app.

<img style="border: 1px solid #000000;" src="https://lh3.googleusercontent.com/ueHuOOYMMb5JrAd57o1XE_7N_1l89atfHX1GGsZ1CMpzVG1k6-ZeXsb6Ppzlu-RVzOi5y2cLzx9VGQ0=w622-h440" width="288" height="204" />

There are four modes: _UniJoystiCle mode_, _D-pad mode_, _Gyruss mode_ and _Linear mode_.

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

* For MFi controllers, try here: [AfterPad](https://afterpad.com/mficontrollers/">https://afterpad.com/mficontrollers/). The [Nimbus SteelSeries](https://afterpad.com/steelseries-nimbus-the-afterpad-review/) is good. DO NOT get the Status SteelSeries.
* For iCade controlles, try here: [iCade Controllers](http://retrorgb.com/icadecontrollers.html). The ["original" iCade](http://retrorgb.com/icade.html) is good.

### Gyruss mode

<img src="https://lh3.googleusercontent.com/E17hGhA4Ab6wNy1_OnpuzIwfzb3y3nsBFTVZv70gY1UJGSlOiXEGEOxda_YRd43CVQuFlhRbvvbWwIM=w5760-h3600-no" width="344" height="194" />

In this mode you control the black circle that is inside the blue circle strip. You move the black ball by tilting your smartphone, and gravity will do the rest. Press the gray circle at the right for _fire_.

<img class="" src="https://lh3.googleusercontent.com/_mIk_b2YmrhLqCo0IGk81Euw4JsGlK1siu131e67ypI4aCKCxBo59Xu37o-P2QZzeVm3JL1aCFVs928=w5760-h3600-no" width="304" height="59" />

__Compatibility:__

* [Gyruss](http://gamebase64.com/game.php?id=3370&d=18&h=0)
* all the <em>UniJoystiCle mode</em> games
* and games with similar techniques to Gyruss

__Using this mode:__ [YouTube video](https://youtu.be/n2YHoj1pXB8)

### Linear mode

<img class="" src="https://lh3.googleusercontent.com/TcfUvufZWZBwNOpHt9y6_y6MEsXQ90xKGHmvJQAzr0p7kCwuvAfgm4LqLzYp0jDJMphJHG5LhnZQ25k=w5760-h3600-no" width="344" height="194" />

This mode is not meant to play games. Instead, is meant to convert the joysticks signals into a numbers from 0 to 31. Since each joystick has 5 lines (5 bits), we have 5 ^^ 2 == 32 combinations.

__Compatibility:__

* [C64 I.D.IoT.R](https://github.com/ricardoquesada/c64-idiotr), the Intelligent Dimmer for IoT RRRRR

__Using this mode:__ [YouTube video](https://www.youtube.com/watch?v=eKlaUfoTuYQ)

## Assembling the PCB

The hardware as well as the software are open source. So you can build one yourself. The schematic and board files are in [Eagle](https://cadsoft.io/) format:

* board: <a href="https://github.com/ricardoquesada/unijoysticle/blob/unijoysticle-v0.3/schematic/unijoysticle.brd">unijoysticle.brd</a> (v0.2.2)
* schematic: <a href="https://github.com/ricardoquesada/unijoysticle/blob/unijoysticle-v0.3/schematic/unijoysticle.sch">unijoysticle.sch</a> (v0.2.2)

You will need the following components:

* 1 x [NodeMCU ESP8266](http://www.aliexpress.com/item/New-Wireless-module-NodeMcu-Lua-WIFI-Internet-of-Things-development-board-based-ESP8266-with-pcb-Antenna/32303690854.html)
* 2 x [DB9 female](https://www.digikey.com/product-detail/en/assmann-wsw-components/A-DF-09-A-KG-T2S/AE10921-ND/1241800)
* 3 x [4066 IC](https://www.digikey.com/product-detail/en/texas-instruments/SN74HC4066N/296-8329-5-ND/376726)
* 3 x [sockets for the ICs](https://www.digikey.com/product-detail/en/on-shore-technology-inc/SA143000/ED3014-ND/3313545)
* 1 x [barrel jack](https://www.digikey.com/product-detail/en/cui-inc/PJ-002A/CP-002A-ND/96962)
* 1 x [4001 diode](https://www.digikey.com/product-detail/en/micro-commercial-co/1N4001-TP/1N4001-TPMSCT-ND/773688)
* [Pins](http://www.aliexpress.com/item/Free-shppping-3200pin-1x40-Pin-2-54mm-Single-Row-Female-Pin-Header-Connector-80PCS-LOT/1897568598.html)

And solder them. It should be straightforward where to place the components. If not, [ping me](https://twitter.com/ricardoquesada)

The barrel jack and the diode are optional. They are only needed if you want to power the NodeMCU module with an external power source. If so, you can use any DC power that from 5v to 12v. 500mA should be enough. Probably it will work with less as well. It works with a 9v battery, although that might not be the most efficient way to power the NodeMCU module.

If you want to avoid the soldering, you can __[order an already assembled PCB from here](https://retro.moe/unijoysticle/)__

After assembling the PCB, it should look like this:

<img class="" src="https://lh3.googleusercontent.com/yPbYMpfy4th501PObRIPJa0vGIGubWI4YZfMf8zcqMMhFt0tN0teuPUGoLKu244fXXxJ_c4n95gNrQ=w2464-h1670-no" width="468" height="318" />

<img src="https://lh3.googleusercontent.com/EzcB9pZfIeSFlQqB6ONgqUOGPxgydRiCND_uexf3BiooADe20sfCTA-bgKs2NJLEUY6ANjG9HFU8dA=w2528-h1586-no" width="482" height="302" />

## Installing the firmware

* Install [PlatformIO](http://platformio.org)
* Clone the [UniJoystiCle github repo]("https://github.com/ricardoquesada/unijoysticle)
* Connect the ESP8266 module with the USB cable to the computer
* And do `make && make upload`

Example:
```
$ git clone https://github.com/ricardoquesada/unijoysticle.git
$ cd unijoysticle/esp8266_firmware/firmware
$ make
$ make upload
```

## Bugs

* To boot the NodeMCU make sure that it is not connected to the Commodore 64. Otherwise it might not boot. I guess (big guess) it is related to a line that should be floating, but it is not when it is connected to the C64.
