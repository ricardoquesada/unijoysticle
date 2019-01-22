# Unijoysticle v2.0

Unijoysticle + bluetooth.

## About

Use modern bluetooth gamepads / mouse in old computers like the Commodore 64, Atari ST, Amiga 500, Apple 2, Tandy 1000, IBM PC Jr., Atari 800, etc.

## Status

Early develop stages. PoC working on PC. Need the rest.

- firmware works on PC.
- missing all the rest.


## Want to help ?

- Download libusb: https://libusb.info/ (needed to test on PC with bluetooth dongle).
- Download BlueKitchen: https://github.com/bluekitchen/btstack
- Download Unijoysticle code: https://gitlab.com/ricardoquesada/unijoysticle
- Get any bluetooth gamepad (iOS, Android, Xbox tested. PS4 not supported ATM)
- For ESP32:
  - Download ESP-IDF: https://github.com/espressif/esp-idf
  - Get an ESP32 module, like [the MH-ET Live][1] which is pin-compatible with [Wemos D1 Mini][2] (used by Unijoysticle v0.4.2 PCB).

### Compile BTstack

- Install the Bluetooth USB dongle on your PC/Mac/Linux
- libusb: Compile and install. Don't use master if you are on macOS. Use 1.0.22 or earlier
- bluekitchen: Compile the libusb port to make sure everything works.

```sh
$ cd src/btstack/port/libusb
$ make
$ ./gap_inquiry
```

Put your gamepad in bluetooth discovery mode and you should see it.


### Compile Unijoysticle

```sh
$ cd src/unijoysticle/firmware/esp32/tools
$ make
./unijoysticle
```

Put the gamepad in discovery mode. The gamepad should be recognized and when you press buttons, you should see them on the console.

## What works:

- PC + Bluetooth dongle for the moment
- It supports up until 4 gamepads at the same time.
- Discovery/Connect/Disconnect/Reconnects works
- Parses HID and identifies all possible gamepad buttons / pads / hats.

## What's missing

### Firmware:
- Create mapping code. Something that correctly maps the different gamepad buttons to the desired ones.
- Support Class / Extended modes:
   - Mode Classic: traditional modes with 1 or 2 gamepads (like one or two joysticks)
   - Mode extended: one gamepad controls 2 joysticks
- Test everything on esp32
  - Disable non required features like WiFi that it takes less current.
- Support mouse/pads: ideally a bluetooth mouse could be used.

### Hardware

- it is possible that the board could get current from the c64/atari's joystick ports?
- design new board, if possible one that doesn't use devkits
- lines for the C64 / Atari pots
- PC Jr. / Tandy 1000 / Apple II: investigate, they use analog lines

### Mobile application

- [Optional]: Simple Bluetooth application that connects to the host and allow us to configure it
  and see its status, etc.


[1]: https://www.aliexpress.com/item/MH-ET-LIVE-ESP32-MINI-KIT-WiFi-Bluetooth-Internet-of-Things-development-board-based-ESP8266-Fully/32819107932.html
[2]: https://wiki.wemos.cc/products:d1:d1_mini
