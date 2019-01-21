# Unijoysticle v2.0


Unijoysticle + bluetooth.

Current status:

- firmware works on PC
- missing all the rest.


## Wanna help?.

- Download libusb: https://libusb.info/
- Download BlueKitchen: https://github.com/bluekitchen/btstack
- Download Unijoysticle code: https://gitlab.com/ricardoquesada/unijoysticle
- Get any bluetooth gamepad (iOS, Android, Xbox tested. PS4 not supported ATM)
- For ESP32:
  - Download ESP-IDF: https://github.com/espressif/esp-idf
  - Get an ESP32 module, like [the MH-ET Live][1] which is compatible with Unijoysticle v1


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


### Compile unijoysticle

```sh
$ cd src/unijoysticle/firmware/esp32/tools
$ make
./unijoysticle
```

The gamepad should be recognized and when you press buttons, you should see them on the console.

## What works:

- PC only for the moment
- It supports up until 4 gamepads at the sametime.
- Discovery/Connect/Disconnect/Reconnects works


## What's missing


### firmware:
- Create mapping code. Something that correctly maps the different controllers to the desired code
- support multi gamepad / single gamepad modes:
   - Mode Classic: traditional modes with 1 or 2 gamepads (like one or two joysticks)
   - Mode extended: one gamepad emulates 2 joysticks
- test everything on esp32
  - disable non required features like WiFi
- Support mouse/pads: ideally a bluetooth mouse could be used.

### hardware:

- it is possible that the board could get current from the c64/atari's joystick ports?
- design new board, if possible one that doesn't use devkits
- add lines for pots so that mouse/pad could be emulated

### Mobile application

- [Optional]: Simple Bluetooth application that connects to the host and allow us to configure it
  and see its status, etc.


[1]: https://www.aliexpress.com/item/MH-ET-LIVE-ESP32-MINI-KIT-WiFi-Bluetooth-Internet-of-Things-development-board-based-ESP8266-Fully/32819107932.html
