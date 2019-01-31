/****************************************************************************
http://retro.moe/unijoysticle

Copyright 2019 Ricardo Quesada

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

#include "my_hid_device.h"

#define MAX_DEVICES                 8

#define MASK_COD_MAJOR_PERIPHERAL   0x0500   // 0b0000_0101_0000_0000
#define MASK_COD_MAJOR_AUDIO        0x0400   // 0b0000_0100_0000_0000
#define MASK_COD_MINOR_MASK         0x00FC   //             1111_1100
#define MASK_COD_MINOR_POINT_DEVICE 0x0080   //             1000_0000
#define MASK_COD_MINOR_GAMEPAD      0x0008   //             0000_1000
#define MASK_COD_MINOR_JOYSTICK     0x0004   //             0000_0100
#define MASK_COD_MINOR_HANDS_FREE   0x0008   //             0000_1000

static my_hid_device_t devices[MAX_DEVICES];
static my_hid_device_t* current_device = NULL;
static int device_count = 0;
static const bd_addr_t zero_addr = {0,0,0,0,0,0};

void my_hid_device_init(void) {
    memset(devices, 0, sizeof(devices));
}

my_hid_device_t* my_hid_device_create(void) {
    for (int j=0; j< MAX_DEVICES; j++){
        if (bd_addr_cmp(devices[j].address, zero_addr) == 0){
            // FIXME: hack. Assign gamepad to joystick based
            // on this:
            //   1st device that is gamepad to port B
            //   1st device that is gamepad to port A
            //   2nd device, regardless what it is, to available port
            devices[j].joystick_port = JOYSTICK_PORT_B;
            return &devices[j];
        }
    }
    return NULL;
}

my_hid_device_t* my_hid_device_get_instance_for_cid(uint16_t cid){
    if (cid == 0)
        return NULL;
    for (int i=0; i<MAX_DEVICES; i++) {
        if (devices[i].hid_interrupt_cid == cid || devices[i].hid_control_cid == cid) {
            return &devices[i];
        }
    }
    return NULL;
}

my_hid_device_t* my_hid_device_get_instance_for_address(bd_addr_t addr) {
    int idx = my_hid_device_get_index_for_address(addr);
    if (idx == -1)
        return NULL;
    return &devices[idx];
}

int my_hid_device_get_index_for_address(bd_addr_t addr){
    for (int j=0; j< MAX_DEVICES; j++){
        if (bd_addr_cmp(addr, devices[j].address) == 0){
            return j;
        }
    }
    return -1;
}

void my_hid_device_remove_entry_with_channel(uint16_t channel) {
    if (channel == 0)
        return;
    for (int i=0; i<MAX_DEVICES; i++) {
        if (devices[i].hid_control_cid == channel || devices[i].hid_interrupt_cid == channel) {
            // Just in case the key is outdated. Proves that it fixes some connection/reconnection issues
            // on certain devices.
            gap_drop_link_key_for_bd_addr(devices[i].address);
            memset(&devices[i], 0, sizeof(devices[i]));
            break;
        }
    }
}

void my_hid_device_set_disconnected(my_hid_device_t* device) {
    // Connection oriented
    device->connected = 0;
    device->incoming = 0;
    device->hid_control_cid = 0;
    device->hid_interrupt_cid = 0;
    device->expected_hid_control_psm = 0;
    device->expected_hid_interrupt_psm = 0;

    // Joystick-state oriented
    device->joystick_port = JOYSTICK_PORT_NONE;
    memset(&device->gamepad, 0, sizeof(device->gamepad));
}

void my_hid_device_assign_joystick_port(my_hid_device_t* device) {
    (void)(device);
}

void my_hid_device_request_inquire(void) {
    for (int i=0;i<MAX_DEVICES;i++) {
        // retry remote name request
        if (devices[i].state == REMOTE_NAME_INQUIRED) {
            devices[i].state = REMOTE_NAME_REQUEST;
        }
    }
}
my_hid_device_t* my_hid_device_get_first_device_with_state(int state) {
    for (int i=0;i<device_count;i++) {
        if (devices[i].state == state)
            return &devices[i];
    }
    return NULL;
}

void my_hid_device_set_current_device(my_hid_device_t* device) {
    current_device = device;
}

my_hid_device_t* my_hid_device_get_current_device(void) {
    return current_device;
}

int my_hid_device_is_cod_supported(uint32_t cod) {
    const uint32_t minor_cod = cod & MASK_COD_MINOR_MASK;
    // Joysticks, mice, gamepads are valid.
    if ((cod & MASK_COD_MAJOR_PERIPHERAL) == MASK_COD_MAJOR_PERIPHERAL) {
        // device is a peripheral: keyboard, mouse, joystick, gamepad...
        // but we only care about joysticks and gamepads
        return (minor_cod & MASK_COD_MINOR_GAMEPAD) ||
                (minor_cod & MASK_COD_MINOR_JOYSTICK) ||
                (minor_cod & MASK_COD_MINOR_POINT_DEVICE);
    }

    // TV remote controls are valid as well
    // Amazon TV remote control reports as CoD: 0x00400408
    //    Audio + Telephony : Hands free
    if ((cod & MASK_COD_MAJOR_AUDIO) == MASK_COD_MAJOR_AUDIO) {
        return (minor_cod & MASK_COD_MINOR_HANDS_FREE);
    }
    return 0;
}
