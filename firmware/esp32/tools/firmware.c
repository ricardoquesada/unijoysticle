/*
 * Copyright (C) 2017 BlueKitchen GmbH
 *
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions
 * are met:
 *
 * 1. Redistributions of source code must retain the above copyright
 *    notice, this list of conditions and the following disclaimer.
 * 2. Redistributions in binary form must reproduce the above copyright
 *    notice, this list of conditions and the following disclaimer in the
 *    documentation and/or other materials provided with the distribution.
 * 3. Neither the name of the copyright holders nor the names of
 *    contributors may be used to endorse or promote products derived
 *    from this software without specific prior written permission.
 * 4. Any redistribution, use, or modification is done solely for
 *    personal benefit and not for any commercial purpose or for
 *    monetary gain.
 *
 * THIS SOFTWARE IS PROVIDED BY BLUEKITCHEN GMBH AND CONTRIBUTORS
 * ``AS IS'' AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
 * LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
 * FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL MATTHIAS
 * RINGWALD OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
 * INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
 * BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; LOSS
 * OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED
 * AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY,
 * OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF
 * THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF
 * SUCH DAMAGE.
 *
 * Please inquire about commercial licensing options at
 * contact@bluekitchen-gmbh.com
 *
 */

/*
 * Copyright (C) 2019 Ricardo Quesada
 * Unijoysticle additions based on the following BlueKitchen's test/example files:
 *   - hid_host_test.c
 *   - hid_device.c
 *   - gap_inquire.c
 *   - hid_device_test.c
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btstack_config.h"
#include "btstack.h"

#include "gpio_joy.h"

#define INQUIRY_INTERVAL            5
#define MASK_COD_MAJOR_PERIPHERAL   0x0500   // 0b0000_0101_0000_0000
#define MASK_COD_MAJOR_AUDIO        0x0400   // 0b0000_0100_0000_0000
#define MASK_COD_MINOR_MASK         0x00FC   //             1111_1100
#define MASK_COD_MINOR_POINT_DEVICE 0x0080   //             1000_0000
#define MASK_COD_MINOR_GAMEPAD      0x0008   //             0000_1000
#define MASK_COD_MINOR_JOYSTICK     0x0004   //             0000_0100
#define MASK_COD_MINOR_HANDS_FREE   0x0008   //             0000_1000
#define MAX_ATTRIBUTE_VALUE_SIZE    512      // Apparently PS4 has a 470-bytes report
#define MAX_DEVICES                 8
#define MTU                         100

// SDP
static uint8_t            attribute_value[MAX_ATTRIBUTE_VALUE_SIZE];
static const unsigned int attribute_value_buffer_size = MAX_ATTRIBUTE_VALUE_SIZE;

enum GAMEPAD_STATES {
    GAMEPAD_STATE_HAT = 1 << 0,
    GAMEPAD_STATE_X = 1 << 1,
    GAMEPAD_STATE_Y = 1 << 2,
    GAMEPAD_STATE_Z = 1 << 3,
    GAMEPAD_STATE_RX = 1 << 4,
    GAMEPAD_STATE_RY = 1 << 5,
    GAMEPAD_STATE_RZ = 1 << 6,
    GAMEPAD_STATE_DPAD = 1 << 7,
    GAMEPAD_STATE_BRAKE = 1 << 8,
    GAMEPAD_STATE_ACCELERATOR = 1 << 9,
    GAMEPAD_STATE_BUTTON0 = 1 << 10,
    GAMEPAD_STATE_BUTTON1 = 1 << 11,
    GAMEPAD_STATE_BUTTON2 = 1 << 12,
    GAMEPAD_STATE_BUTTON3 = 1 << 13,
    GAMEPAD_STATE_BUTTON4 = 1 << 14,
    GAMEPAD_STATE_BUTTON5 = 1 << 15,
    GAMEPAD_STATE_BUTTON6 = 1 << 16,
    GAMEPAD_STATE_BUTTON7 = 1 << 17,
    GAMEPAD_STATE_BUTTON8 = 1 << 18,
    GAMEPAD_STATE_BUTTON9 = 1 << 19,
    GAMEPAD_STATE_BUTTON10 = 1 << 20,
    GAMEPAD_STATE_BUTTON11 = 1 << 21,
    GAMEPAD_STATE_BUTTON12 = 1 << 22,
    GAMEPAD_STATE_BUTTON13 = 1 << 23,
    GAMEPAD_STATE_BUTTON14 = 1 << 24,
    GAMEPAD_STATE_BUTTON15 = 1 << 25,
};

typedef struct gamepad {
    // Usage Page: 0x01 (Generic Desktop Controls)
    uint8_t hat;
    int32_t x;
    int32_t y;
    int32_t z;
    int32_t rx;
    int32_t ry;
    int32_t rz;
    uint8_t dpad;

    // Usage Page: 0x02 (Sim controls)
    int32_t     brake;
    int32_t     accelerator;

    // Usage Page: 0x06 (Generic dev controls)
    uint16_t    battery;

    // Usage Page: 0x09 (Button)
    uint32_t    buttons;

    // Misc buttos (from 0x0c (Consumer) and others)
    uint8_t     misc_buttons;

    // updated states
    uint32_t    updated_states;
} gamepad_t;

enum DEVICE_STATE { REMOTE_NAME_REQUEST, REMOTE_NAME_INQUIRED, REMOTE_NAME_FETCHED };
enum JOYSTICK_PORT { JOYSTICK_PORT_NONE, JOYSTICK_PORT_A, JOYSTICK_PORT_B, JOYSTICK_PORT_AB};
typedef struct  {
    bd_addr_t           address;
    hci_con_handle_t    con_handle;
    uint8_t             page_scan_repetition_mode;
    uint16_t            clock_offset;
    uint32_t            cod;
    char                name[240];

    // state
    uint8_t             incoming;
    uint8_t             connected;

    // SDP
    uint8_t             hid_descriptor[MAX_ATTRIBUTE_VALUE_SIZE];
    uint16_t            hid_descriptor_len;

    // Channels
    uint16_t            hid_control_cid;
    uint16_t            hid_interrupt_cid;
    uint16_t            expected_hid_control_psm;         // must be PSM_HID_CONTROL
    uint16_t            expected_hid_interrupt_psm;       // must be PSM_HID_INTERRUPT
    enum DEVICE_STATE   state;

    // gamepad
    gamepad_t           gamepad;
    enum JOYSTICK_PORT  joystick_port;
} my_hid_device_t;

// btstack bug:
// see: https://github.com/bluekitchen/btstack/issues/187
typedef struct {
    int32_t         logical_minimum;
    int32_t         logical_maximum;
    uint16_t        usage_page;
    uint8_t         report_size;
    uint8_t         report_count;
    uint8_t         report_id;
} hid_globals_t;

static my_hid_device_t devices[MAX_DEVICES];
static my_hid_device_t* current_device = NULL;
static int device_count = 0;
static const bd_addr_t zero_addr = {0,0,0,0,0,0};

// Asus
// static const char * remote_addr_string = "54:A0:50:CD:A6:2F";
// static bd_addr_t remote_addr;

static btstack_packet_callback_registration_t hci_event_callback_registration;

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void handle_sdp_client_query_result(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void print_gamepad(gamepad_t* gamepad);
static void print_descriptor_item(hid_descriptor_item_t* item);
static void print_parser_globals(hid_globals_t* globals);
static void hid_host_handle_interrupt_report(my_hid_device_t* device, const uint8_t * report, uint16_t report_len);
static void continue_remote_names(void);
static void start_scan(void);
static int is_device_supported(uint32_t cod);
static my_hid_device_t* my_hid_device_get_instance_for_cid(uint16_t cid);
static int my_hid_device_get_index_for_address(bd_addr_t addr);
static my_hid_device_t* my_hid_device_get_instance_for_address(bd_addr_t addr);
static void my_hid_device_set_disconnected(my_hid_device_t* device);
static my_hid_device_t* my_hid_device_create(void);
static void my_hid_device_incoming_connection(uint8_t *packet, uint16_t channel);
static int my_hid_device_channel_opened(uint8_t* packet, uint16_t channel);
static void my_hid_device_channel_closed(uint8_t* packet, uint16_t channel);
static void my_hid_device_remove_entry_with_channel(uint16_t channel);
static void my_hid_device_assign_joystick_port(my_hid_device_t* device);
static int32_t hid_process_thumbstick(btstack_hid_parser_t* parser, hid_globals_t* globals, uint32_t value);
static uint8_t hid_process_hat(btstack_hid_parser_t* parser, hid_globals_t* globals, uint32_t value);
static void process_usage(my_hid_device_t* device, btstack_hid_parser_t* parser, hid_globals_t* globals, uint16_t usage_page, uint16_t usage, int32_t value);
static void joystick_update(my_hid_device_t* device);

int btstack_main(int argc, const char * argv[]);

static void hid_host_setup(void){

    // enabled EIR
    hci_set_inquiry_mode(INQUIRY_MODE_RSSI_AND_EIR);

    // register for HCI events
    hci_event_callback_registration.callback = &packet_handler;
    hci_add_event_handler(&hci_event_callback_registration);

    l2cap_register_service(packet_handler, PSM_HID_INTERRUPT, MTU, LEVEL_2);
    l2cap_register_service(packet_handler, PSM_HID_CONTROL,   MTU, LEVEL_2);

    // Disable stdout buffering
    setbuf(stdout, NULL);
}

/* @section SDP parser callback
 *
 * @text The SDP parsers retrieves the BNEP PAN UUID as explained in
 * Section [on SDP BNEP Query example](#sec:sdpbnepqueryExample}.
 */


static void handle_sdp_client_query_result(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size) {

    UNUSED(packet_type);
    UNUSED(channel);
    UNUSED(size);

    des_iterator_t  attribute_list_it;
    des_iterator_t  additional_des_it;
    des_iterator_t  prot_it;
    uint8_t*        des_element;
    uint8_t*        element;
    uint32_t        uuid;

    // printf_hexdump(packet, size);

    if (current_device == NULL) {
        printf("ERROR: handle_sdp_client_query_result. current_device = NULL\n");
        return;
    }

    switch (hci_event_packet_get_type(packet)){
        case SDP_EVENT_QUERY_ATTRIBUTE_VALUE:
            if (sdp_event_query_attribute_byte_get_attribute_length(packet) <= attribute_value_buffer_size) {
                attribute_value[sdp_event_query_attribute_byte_get_data_offset(packet)] = sdp_event_query_attribute_byte_get_data(packet);
                if ((uint16_t)(sdp_event_query_attribute_byte_get_data_offset(packet)+1) == sdp_event_query_attribute_byte_get_attribute_length(packet)) {
                    switch(sdp_event_query_attribute_byte_get_attribute_id(packet)) {
                        case BLUETOOTH_ATTRIBUTE_PROTOCOL_DESCRIPTOR_LIST:
                            for (des_iterator_init(&attribute_list_it, attribute_value); des_iterator_has_more(&attribute_list_it); des_iterator_next(&attribute_list_it)) {
                                if (des_iterator_get_type(&attribute_list_it) != DE_DES) continue;
                                des_element = des_iterator_get_element(&attribute_list_it);
                                des_iterator_init(&prot_it, des_element);
                                element = des_iterator_get_element(&prot_it);
                                if (de_get_element_type(element) != DE_UUID) continue;
                                uuid = de_get_uuid32(element);
                                switch (uuid){
                                    case BLUETOOTH_PROTOCOL_L2CAP:
                                        if (!des_iterator_has_more(&prot_it)) continue;
                                        des_iterator_next(&prot_it);
                                        de_element_get_uint16(des_iterator_get_element(&prot_it), &current_device->expected_hid_control_psm);
                                        printf("SDP HID Control PSM: 0x%04x\n", (int) current_device->expected_hid_control_psm);
                                        break;
                                    default:
                                        break;
                                }
                            }
                            break;
                        case BLUETOOTH_ATTRIBUTE_ADDITIONAL_PROTOCOL_DESCRIPTOR_LISTS:
                            for (des_iterator_init(&attribute_list_it, attribute_value); des_iterator_has_more(&attribute_list_it); des_iterator_next(&attribute_list_it)) {
                                if (des_iterator_get_type(&attribute_list_it) != DE_DES) continue;
                                des_element = des_iterator_get_element(&attribute_list_it);
                                for (des_iterator_init(&additional_des_it, des_element); des_iterator_has_more(&additional_des_it); des_iterator_next(&additional_des_it)) {
                                    if (des_iterator_get_type(&additional_des_it) != DE_DES) continue;
                                    des_element = des_iterator_get_element(&additional_des_it);
                                    des_iterator_init(&prot_it, des_element);
                                    element = des_iterator_get_element(&prot_it);
                                    if (de_get_element_type(element) != DE_UUID) continue;
                                    uuid = de_get_uuid32(element);
                                    switch (uuid){
                                        case BLUETOOTH_PROTOCOL_L2CAP:
                                            if (!des_iterator_has_more(&prot_it)) continue;
                                            des_iterator_next(&prot_it);
                                            de_element_get_uint16(des_iterator_get_element(&prot_it), &current_device->expected_hid_interrupt_psm);
                                            printf("SDP HID Interrupt PSM: 0x%04x\n", (int) current_device->expected_hid_interrupt_psm);
                                            break;
                                        default:
                                            break;
                                    }
                                }
                            }
                            break;
                        case BLUETOOTH_ATTRIBUTE_HID_DESCRIPTOR_LIST:
                            for (des_iterator_init(&attribute_list_it, attribute_value); des_iterator_has_more(&attribute_list_it); des_iterator_next(&attribute_list_it)) {
                                if (des_iterator_get_type(&attribute_list_it) != DE_DES) continue;
                                des_element = des_iterator_get_element(&attribute_list_it);
                                for (des_iterator_init(&additional_des_it, des_element); des_iterator_has_more(&additional_des_it); des_iterator_next(&additional_des_it)) {
                                    if (des_iterator_get_type(&additional_des_it) != DE_STRING) continue;
                                    element = des_iterator_get_element(&additional_des_it);
                                    const uint8_t * descriptor = de_get_string(element);
                                    current_device->hid_descriptor_len = de_get_data_size(element);
                                    printf("SDP HID Descriptor (%d):\n", current_device->hid_descriptor_len);
                                    memcpy(current_device->hid_descriptor, descriptor, current_device->hid_descriptor_len);
                                    printf_hexdump(current_device->hid_descriptor, current_device->hid_descriptor_len);
                                }
                            }
                            break;
                        default:
                            break;
                    }
                }
            } else {
                fprintf(stderr, "SDP attribute value buffer size exceeded: available %d, required %d\n", attribute_value_buffer_size, sdp_event_query_attribute_byte_get_attribute_length(packet));
            }
            break;

        case SDP_EVENT_QUERY_RFCOMM_SERVICE:
            printf("SDP_EVENT_QUERY_RFCOMM_SERVICE\n");
            break;
        case SDP_EVENT_QUERY_ATTRIBUTE_BYTE:
            printf("SDP_EVENT_QUERY_ATTRIBUTE_BYTE");
            break;
        case SDP_EVENT_QUERY_SERVICE_RECORD_HANDLE:
            printf("SDP_EVENT_QUERY_SERVICE_RECORD_HANDLE\n");
            break;
        case SDP_EVENT_QUERY_COMPLETE:
            printf("SDP_EVENT_QUERY_COMPLETE\n");
            if (current_device->expected_hid_control_psm != PSM_HID_CONTROL) {
                printf("Invalid Control PSM missing. Expecting = 0x%04x, got = 0x%04x\n", PSM_HID_CONTROL, current_device->expected_hid_control_psm);
                break;
            }
            if (current_device->expected_hid_interrupt_psm != PSM_HID_INTERRUPT) {
                printf("Invalid Control PSM missing. Expecting = 0x%04x, got = 0x%04x\n", PSM_HID_INTERRUPT, current_device->expected_hid_interrupt_psm);
                break;
            }
            printf("Setup HID completed.\n");
            // We assume that connection + HID discovery is done. set current_device to NULL so that another
            // SDP query can be done.
            current_device = NULL;
            break;
    }
}

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    uint8_t   event;
    bd_addr_t event_addr;
    uint8_t   status;
    int       index;
    my_hid_device_t* device;

    switch (packet_type) {
    case HCI_EVENT_PACKET:
        event = hci_event_packet_get_type(packet);
        switch (event) {
        /* @text When BTSTACK_EVENT_STATE with state HCI_STATE_WORKING
            * is received and the example is started in client mode, the remote SDP HID query is started.
            */
        case BTSTACK_EVENT_STATE:
            if (btstack_event_state_get_state(packet) == HCI_STATE_WORKING){
                printf("Start SDP HID query for remote HID Device.\n");
                start_scan();
            }
            break;

        case HCI_EVENT_PIN_CODE_REQUEST:
            // inform about pin code request
            printf("Pin code request - using '0000'\n");
            hci_event_pin_code_request_get_bd_addr(packet, event_addr);
            gap_pin_code_response(event_addr, "0000");
            break;

        case HCI_EVENT_USER_CONFIRMATION_REQUEST:
            // inform about user confirmation request
            printf("SSP User Confirmation Request with numeric value '%"PRIu32"'\n", little_endian_read_32(packet, 8));
            printf("SSP User Confirmation Auto accept\n");
            break;

        case HCI_EVENT_HID_META:
            printf("HCI_EVENT_HID_META\n");
            switch (hci_event_hid_meta_get_subevent_code(packet)){
            case HID_SUBEVENT_CONNECTION_OPENED:
                status = hid_subevent_connection_opened_get_status(packet);
                if (status) {
                    // outgoing connection failed
                    printf("Connection failed, status 0x%x\n", status);
                    return;
                }
                hid_subevent_connection_opened_get_bd_addr(packet, devices[0].address);
                uint16_t hid_cid = hid_subevent_connection_opened_get_hid_cid(packet);
                printf("HID Connected to %s - %d\n", bd_addr_to_str(devices[0].address), hid_cid);
                break;
            case HID_SUBEVENT_CONNECTION_CLOSED:
                printf("HID Disconnected\n");
                break;
            case HID_SUBEVENT_SUSPEND:
                printf("HID Suspend\n");
                break;
            case HID_SUBEVENT_EXIT_SUSPEND:
                printf("HID Exit Suspend\n");
                break;
            case HID_SUBEVENT_CAN_SEND_NOW:
                printf("HID_SUBEVENT_CAN_SEND_NOW\n");
                break;
            default:
                break;
            }
        case L2CAP_EVENT_INCOMING_CONNECTION:
            my_hid_device_incoming_connection(packet, channel);
            break;
        case L2CAP_EVENT_CHANNEL_OPENED:
            if (my_hid_device_channel_opened(packet, channel) != 0)
                my_hid_device_remove_entry_with_channel(channel);
            break;
        case L2CAP_EVENT_CHANNEL_CLOSED:
            my_hid_device_channel_closed(packet, channel);
            break;
        // GAP related
        case GAP_EVENT_INQUIRY_RESULT:
            // print info
            gap_event_inquiry_result_get_bd_addr(packet, event_addr);
            uint8_t page_scan_repetition_mode = gap_event_inquiry_result_get_page_scan_repetition_mode(packet);
            uint16_t clock_offset = gap_event_inquiry_result_get_clock_offset(packet);
            uint32_t cod = gap_event_inquiry_result_get_class_of_device(packet);

            printf("Device found: %s ",  bd_addr_to_str(event_addr));
            printf("with COD: 0x%06x, ", (unsigned int) cod);
            printf("pageScan %d, ",      page_scan_repetition_mode);
            printf("clock offset 0x%04x", clock_offset);
            if (gap_event_inquiry_result_get_rssi_available(packet)){
                printf(", rssi %d dBm", (int8_t) gap_event_inquiry_result_get_rssi(packet));
            }

            if (is_device_supported(cod)) {
                index = my_hid_device_get_index_for_address(event_addr);
                if (index >= 0) {
                    printf(" ...device already in our list\n");
                    break;   // already in our list
                }

                device = my_hid_device_create();
                if (device == NULL) {
                    printf("\nERROR: no more available device slots\n");
                    break;
                }
                memcpy(device->address, event_addr, 6);
                device->page_scan_repetition_mode = page_scan_repetition_mode;
                device->clock_offset = clock_offset;
                device->cod = cod;
                // my_hid_device_assign_joystick_port(device);
                if (gap_event_inquiry_result_get_name_available(packet)){
                    int name_len = gap_event_inquiry_result_get_name_len(packet);
                    memcpy(device->name, gap_event_inquiry_result_get_name(packet), name_len);
                    device->name[name_len] = 0;
                    printf(", name '%s'", device->name);
                    device->state = REMOTE_NAME_FETCHED;;
                } else {
                    device->state = REMOTE_NAME_REQUEST;
                }
                printf("\n");
                status = l2cap_create_channel(packet_handler, device->address, PSM_HID_CONTROL, 48, &device->hid_control_cid);
                if (status){
                    printf("\nConnecting to HID Control failed: 0x%02x", status);
                }
            }
            printf("\n");
            break;
        case GAP_EVENT_INQUIRY_COMPLETE:
            for (int i=0;i<MAX_DEVICES;i++) {
                // retry remote name request
                if (devices[i].state == REMOTE_NAME_INQUIRED) {
                    devices[i].state = REMOTE_NAME_REQUEST;
                }
            }
            continue_remote_names();
            break;

        case HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE:
            reverse_bd_addr(&packet[3], event_addr);
            index = my_hid_device_get_index_for_address(event_addr);
            if (index >= 0) {
                if (packet[2] == 0) {
                    printf("Name: '%s'\n", &packet[9]);
                    devices[index].state = REMOTE_NAME_FETCHED;
                } else {
                    printf("Failed to get name: page timeout\n");
                }
            }
            continue_remote_names();
            break;

        default:
            break;
        }
        break;
    case L2CAP_DATA_PACKET:
        // for now, just dump incoming data
        device = my_hid_device_get_instance_for_cid(channel);
        if (device == NULL) {
            printf("Invalid cid: 0x%04x\n", channel);
            break;
        }
        if (channel == device->hid_interrupt_cid){
            printf("HID Interrupt Packet: ");
            printf_hexdump(packet, size);
            hid_host_handle_interrupt_report(device, packet,  size);
            print_gamepad(&device->gamepad);
        } else if (channel == device->hid_control_cid){
            printf("HID Control\n");
            printf_hexdump(packet, size);
        } else {
            printf("Packet from unknown cid: 0x%04x\n", channel);
            printf_hexdump(packet, size);
            break;
        }
    default:
        break;
    }
}

static int is_device_supported(uint32_t cod) {
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

static int has_more_remote_name_requests(void){
    int i;
    for (i=0;i<device_count;i++) {
        if (devices[i].state == REMOTE_NAME_REQUEST) return 1;
    }
    return 0;
}

static void do_next_remote_name_request(void){
    int i;
    for (i=0;i<device_count;i++) {
        // remote name request
        if (devices[i].state == REMOTE_NAME_REQUEST){
            devices[i].state = REMOTE_NAME_INQUIRED;
            printf("Get remote name of %s...\n", bd_addr_to_str(devices[i].address));
            gap_remote_name_request( devices[i].address, devices[i].page_scan_repetition_mode,  devices[i].clock_offset | 0x8000);
            return;
        }
    }
}

static void continue_remote_names(void){
    if (has_more_remote_name_requests()){
        do_next_remote_name_request();
        return;
    }
    start_scan();
}

static void start_scan(void){
    printf("Starting inquiry scan..\n");
    gap_inquiry_start(INQUIRY_INTERVAL);
}


/*
 * @section HID Report Handler
 *
 * @text Use BTstack's compact HID Parser to process incoming HID Report
 * Iterate over all fields and process fields with usage page = 0x07 / Keyboard
 * Check if SHIFT is down and process first character (don't handle multiple key presses)
 *
 */
enum {
    DPAD_UP     = 1 << 0,
    DPAD_DOWN   = 1 << 1,
    DPAD_RIGHT  = 1 << 2,
    DPAD_LEFT   = 1 << 3,
};

enum {
    MISC_AC_SEARCH  = 1 << 0,
    MISC_AC_HOME    = 1 << 1,
    MISC_AC_BACK    = 1 << 2,

    MISC_SYS_MAIN_MENU = 1 << 3,
};

static void print_gamepad(gamepad_t *gamepad) {
    printf("(0x%04x) x=%d, y=%d, z=%d, rx=%d, ry=%d, rz=%d, hat=0x%02x, dpad=0x%02x, accel=%d, brake=%d, buttons=0x%08x, misc=0x%02x\n",
            gamepad->updated_states,
            gamepad->x, gamepad->y, gamepad->z,
            gamepad->rx, gamepad->ry, gamepad->rz,
            gamepad->hat,
            gamepad->dpad,
            gamepad->accelerator, gamepad->brake,
            gamepad->buttons,
            gamepad->misc_buttons
          );

    // gpio_set_level(GPIO_NUM_23, g_gamepad.buttons[1] != 0);
}

static void print_descriptor_item(hid_descriptor_item_t* item) {
    printf("val=0x%04x, size=%d, type=%d, tag=%d, data_size=%d\n",
        item->item_value, item->item_size, item->item_type, item->item_tag, item->data_size);
}

static void print_parser_globals(hid_globals_t* globals) {
    printf("log_min=%d, log_max=%d, usage_page=%d, rep_size=%d, rep_cnt=%d, rep_id=%d\n",
        globals->logical_minimum,
        globals->logical_maximum,
        globals->usage_page,
        globals->report_size,
        globals->report_count,
        globals->report_id);
}

static void hid_host_handle_interrupt_report(my_hid_device_t* device, const uint8_t * report, uint16_t report_len) {
    // check if HID Input Report
    if (report_len < 1) return;
    if (*report != 0xa1) return;
    report++;
    report_len--;

    btstack_hid_parser_t parser;
    hid_globals_t globals;
    btstack_hid_parser_init(&parser, device->hid_descriptor, device->hid_descriptor_len, HID_REPORT_TYPE_INPUT, report, report_len);
    device->gamepad.updated_states = 0;
    while (btstack_hid_parser_has_more(&parser)){
        uint16_t usage_page;
        uint16_t usage;
        int32_t  value;

        // Save globals, otherwise they are going to get destroyed by btstack_hid_parser_get_field()
        globals.logical_minimum = parser.global_logical_minimum;
        globals.logical_maximum = parser.global_logical_maximum;
        globals.report_count = parser.global_report_count;
        globals.report_id = parser.global_report_id;
        globals.report_size = parser.global_report_size;
        globals.usage_page = parser.global_usage_page;

        btstack_hid_parser_get_field(&parser, &usage_page, &usage, &value);

        // printf("usage_page = 0x%04x, usage = 0x%04x, value = 0x%x - ", usage_page, usage, value);
        process_usage(device, &parser, &globals, usage_page, usage, value);
    }
    joystick_update(device);
}

static void process_usage(my_hid_device_t* device, btstack_hid_parser_t* parser, hid_globals_t* globals, uint16_t usage_page, uint16_t usage, int32_t value) {
    // print_parser_globals(globals);
    switch (usage_page) {
    case 0x01:  // Generic Desktop controls
        switch (usage) {
        case 0x30:  // x
            device->gamepad.x = hid_process_thumbstick(parser, globals, value);
            device->gamepad.updated_states |= GAMEPAD_STATE_X;
            break;
        case 0x31:  // y
            device->gamepad.y = hid_process_thumbstick(parser, globals, value);
            device->gamepad.updated_states |= GAMEPAD_STATE_Y;
            break;
        case 0x32:  // z
            device->gamepad.z = hid_process_thumbstick(parser, globals, value);
            device->gamepad.updated_states |= GAMEPAD_STATE_Z;
            break;
        case 0x33:  // rx
            device->gamepad.rx = hid_process_thumbstick(parser, globals, value);
            device->gamepad.updated_states |= GAMEPAD_STATE_RX;
            break;
        case 0x34:  // ry
            device->gamepad.ry = hid_process_thumbstick(parser, globals, value);
            device->gamepad.updated_states |= GAMEPAD_STATE_RY;
            break;
        case 0x35:  // rz
            device->gamepad.rz = hid_process_thumbstick(parser, globals, value);
            device->gamepad.updated_states |= GAMEPAD_STATE_RZ;
            break;
        case 0x39:  // switch hat
            device->gamepad.hat = hid_process_hat(parser, globals, value);
            device->gamepad.updated_states |= GAMEPAD_STATE_HAT;
            break;
        case 0x85: // system main menu
            if (value)
                device->gamepad.misc_buttons |= MISC_SYS_MAIN_MENU;
            else
                device->gamepad.misc_buttons &= ~MISC_SYS_MAIN_MENU;
            break;
        case 0x90:  // dpad up
            if (value)
                device->gamepad.dpad |= DPAD_UP;
            else
                device->gamepad.dpad &= ~DPAD_UP;
            device->gamepad.updated_states |= GAMEPAD_STATE_DPAD;
            break;
        case 0x91:  // dpad down
            if (value)
                device->gamepad.dpad |= DPAD_DOWN;
            else
                device->gamepad.dpad &= ~DPAD_DOWN;
            device->gamepad.updated_states |= GAMEPAD_STATE_DPAD;
            break;
        case 0x92:  // dpad right
            if (value)
                device->gamepad.dpad |= DPAD_RIGHT;
            else
                device->gamepad.dpad &= ~DPAD_RIGHT;
            device->gamepad.updated_states |= GAMEPAD_STATE_DPAD;
            break;
        case 0x93:  // dpad left
            if (value)
                device->gamepad.dpad |= DPAD_LEFT;
            else
                device->gamepad.dpad &= ~DPAD_LEFT;
            device->gamepad.updated_states |= GAMEPAD_STATE_DPAD;
            break;
        default:
            printf("Unsupported usage: 0x%04x for page: 0x%04x. value=0x%x\n", usage, usage_page, value);
            break;
        }
        break;
    case 0x02:  // Simulation controls
        switch (usage) {
        case 0xc4:  // accelerator
            device->gamepad.accelerator = value;
            device->gamepad.updated_states |= GAMEPAD_STATE_ACCELERATOR;
            break;
        case 0xc5:  // brake
            device->gamepad.brake = value;
            device->gamepad.updated_states |= GAMEPAD_STATE_BRAKE;
            break;
        default:
            printf("Unsupported usage: 0x%04x for page: 0x%04x. value=0x%x\n", usage, usage_page, value);
            break;
        };
        break;
    case 0x06: // Generic Device Controls Page
        switch (usage) {
        case 0x20: // Battery Strength
            device->gamepad.battery = value;
            break;
        default:
            printf("Unsupported usage: 0x%04x for page: 0x%04x. value=0x%x\n", usage, usage_page, value);
            break;
        }
        break;
    case 0x07:  // Keypad / Keyboard
        // FIXME: It is unlikely a device has both a dpap a keyboard, so we report certain keys
        // as dpad, just to avoid having a entry entry in the gamepad_t type.
        switch (usage) {
        case 0x4f:  // Right arrow
        case 0x5e:  // Keypad right arrow
            if (value)
                device->gamepad.dpad |= DPAD_RIGHT;
            else
                device->gamepad.dpad &= ~DPAD_RIGHT;
            device->gamepad.updated_states |= GAMEPAD_STATE_DPAD;
            break;
        case 0x50:  // Left arrow
        case 0x5c:  // Keypad left arrow
            if (value)
                device->gamepad.dpad |= DPAD_LEFT;
            else
                device->gamepad.dpad &= ~DPAD_LEFT;
            device->gamepad.updated_states |= GAMEPAD_STATE_DPAD;
            break;
        case 0x51:  // Down arrow
        case 0x5a:  // Keypad down arrow
            if (value)
                device->gamepad.dpad |= DPAD_DOWN;
            else
                device->gamepad.dpad &= ~DPAD_DOWN;
            device->gamepad.updated_states |= GAMEPAD_STATE_DPAD;
            break;
        case 0x52:  // Up arrow
        case 0x60:  // Keypad up arrow
            if (value)
                device->gamepad.dpad |= DPAD_UP;
            else
                device->gamepad.dpad &= ~DPAD_UP;
            device->gamepad.updated_states |= GAMEPAD_STATE_DPAD;
            break;
        case 0x1d:  // z
        case 0x28:  // Enter
        case 0x2c:  // spacebar
        case 0x58:  // Keypad enter
            if (value)
                device->gamepad.buttons |= (1 << 0);
            else
                device->gamepad.buttons &= ~(1 << 0);
            device->gamepad.updated_states |= GAMEPAD_STATE_BUTTON0;
            break;
        default:
            printf("Unsupported usage: 0x%04x for page: 0x%04x. value=0x%x\n", usage, usage_page, value);
            break;
        }
        break;
    case 0x09:  // Button
    {
        // we start with usage - 1 since "button 0" seems that is not being used
        // and we only support 32 buttons.
        const uint16_t button_idx = usage-1;
        if (button_idx < 16) {
            if (value)
                device->gamepad.buttons |= (1 << button_idx);
            else
                device->gamepad.buttons &= ~(1 << button_idx);
            device->gamepad.updated_states |= (GAMEPAD_STATE_BUTTON0 << button_idx);
        } else {
            printf("Unsupported usage: 0x%04x for page: 0x%04x. value=0x%x\n", usage, usage_page, value);
        }
        break;
    }
    case 0x0c:  // Consumer
        switch (usage) {
        case 0x221:     // search
            if (value)
                device->gamepad.misc_buttons |= MISC_AC_SEARCH;
            else
                device->gamepad.misc_buttons &= ~MISC_AC_SEARCH;
        case 0x0223:    // home
            if (value)
                device->gamepad.misc_buttons |= MISC_AC_HOME;
            else
                device->gamepad.misc_buttons &= ~MISC_AC_HOME;
            break;
        case 0x0224:    // back
            if (value)
                device->gamepad.misc_buttons |= MISC_AC_BACK;
            else
                device->gamepad.misc_buttons &= ~MISC_AC_BACK;
            break;
        default:
            printf("Unsupported usage: 0x%04x for page: 0x%04x. value=0x%x\n", usage, usage_page, value);
            break;
        }
        break;

    // unknown usage page
    default:
        printf("Unsupported usage: 0x%04x for page: 0x%04x. value=0x%x\n", usage, usage_page, value);
        break;
    }
}

// Converts gamepad to joystick.
static void joystick_update(my_hid_device_t* device) {
    if (device->joystick_port == JOYSTICK_PORT_NONE)
        return;

    // FIXME: Add support for JOYSTICK_PORT_AB.
    joystick_t joy;

    // reset state
    memset(&joy, 0, sizeof(joy));

    const gamepad_t* gp = &device->gamepad;
    if (gp->updated_states & GAMEPAD_STATE_HAT) {
        switch (gp->hat) {
        case 0xff:
            // joy.up = joy.down = joy.left = joy.right = 0;
            break;
        case 0:
            joy.up |= 1;
            break;
        case 1:
            joy.up |= 1;
            joy.right |= 1;
            break;
        case 2:
            joy.right |= 1;
            break;
        case 3:
            joy.right |= 1;
            joy.down |= 1;
            break;
        case 4:
            joy.down |= 1;
            break;
        case 5:
            joy.down |= 1;
            joy.left |= 1;
            break;
        case 6:
            joy.left |= 1;
            break;
        case 7:
            joy.left |= 1;
            joy.up |= 1;
            break;
        default:
            printf("Error parsing hat values\n");
            break;
        }
    }

    if (gp->updated_states & GAMEPAD_STATE_DPAD) {
        if (gp->dpad & 0x01)
            joy.up |= 1;
        if (gp->dpad & 0x02)
            joy.down |= 1;
        if (gp->dpad & 0x04)
            joy.right |= 1;
        if (gp->dpad & 0x08)
            joy.left |= 1;
    }

    if (gp->updated_states & GAMEPAD_STATE_BUTTON0) {
        joy.fire |= gp->buttons & 1;
    }

    if (gp->updated_states & GAMEPAD_STATE_X) {
        joy.left |= (gp->x < -64);
        joy.right |= (gp->x > 64);
    }
    if (gp->updated_states & GAMEPAD_STATE_Y) {
        joy.down |= (gp->y < -64);
        joy.up |= (gp->y > 64);
    }

    printf("joy state: 0x%04x\n", gp->updated_states);
    // FIXME: Add support for JOYSTICK_PORT_AB.
    if (device->joystick_port == JOYSTICK_PORT_A)
        gpio_joy_update_port_a(&joy);
    else
        gpio_joy_update_port_b(&joy);
}

// Converts a possible value between (0, x) to (-x/2, x/2)
static int32_t hid_process_thumbstick(btstack_hid_parser_t* parser, hid_globals_t* globals, uint32_t value) {
    UNUSED(parser);
    return value - (globals->logical_maximum - globals->logical_minimum) / 2 - globals->logical_minimum;
}

static uint8_t hid_process_hat(btstack_hid_parser_t* parser, hid_globals_t* globals, uint32_t value) {
    UNUSED(parser);
    // Assumes if value is outside valid range, then it is a "null value"
    if (value < globals->logical_minimum || value > globals->logical_maximum)
        return 0xff;
    // 0 should be the first value for hat, meaning that 0 is the "up" position.
    return value - globals->logical_minimum;
}


// Hid device related
static my_hid_device_t* my_hid_device_get_instance_for_cid(uint16_t cid){
    if (cid == 0)
        return NULL;
    for (int i=0; i<MAX_DEVICES; i++) {
        if (devices[i].hid_interrupt_cid == cid || devices[i].hid_control_cid == cid) {
            return &devices[i];
        }
    }
    return NULL;
}

static int my_hid_device_get_index_for_address(bd_addr_t addr){
    for (int j=0; j< MAX_DEVICES; j++){
        if (bd_addr_cmp(addr, devices[j].address) == 0){
            return j;
        }
    }
    return -1;
}

static my_hid_device_t* my_hid_device_get_instance_for_address(bd_addr_t addr) {
    int idx = my_hid_device_get_index_for_address(addr);
    if (idx == -1)
        return NULL;
    return &devices[idx];
}

static my_hid_device_t* my_hid_device_create(void) {
    for (int j=0; j< MAX_DEVICES; j++){
        if (bd_addr_cmp(devices[j].address, zero_addr) == 0){
            // FIXME: hack
            devices[j].joystick_port = JOYSTICK_PORT_B;
            return &devices[j];
        }
    }
    return NULL;
}

static void my_hid_device_incoming_connection(uint8_t *packet, uint16_t channel) {
    bd_addr_t event_addr;
    my_hid_device_t* device;
    uint16_t local_cid;
    uint16_t remote_cid;
    uint16_t psm;
    hci_con_handle_t handle;

    psm = l2cap_event_incoming_connection_get_psm(packet);
    handle = l2cap_event_incoming_connection_get_handle(packet);
    local_cid = l2cap_event_incoming_connection_get_local_cid(packet);
    remote_cid = l2cap_event_incoming_connection_get_remote_cid(packet);

    printf("L2CAP_EVENT_INCOMING_CONNECTION (psm=0x%04x, local_cid=0x%04x, remote_cid=0x%04x, handle=0x%04x, channel=0x%04x\n", psm, local_cid, remote_cid, handle, channel);
    switch (psm) {
        case PSM_HID_CONTROL:
            l2cap_event_incoming_connection_get_address(packet, event_addr);
            device = my_hid_device_get_instance_for_address(event_addr);
            if (device == NULL) {
                device = my_hid_device_create();
                if (device == NULL) {
                    printf("ERROR: no more available free devices\n");
                    l2cap_decline_connection(channel);
                    break;
                }
                memcpy(device->address, event_addr, 6);
            }
            l2cap_accept_connection(channel);
            device->con_handle = l2cap_event_incoming_connection_get_handle(packet);
            device->incoming = 1;
            device->hid_control_cid = channel;
            break;
        case PSM_HID_INTERRUPT:
            l2cap_event_incoming_connection_get_address(packet, event_addr);
            device = my_hid_device_get_instance_for_address(event_addr);
            if (device == NULL) {
                printf("Could not find device for PSM_HID_INTERRUPT = 0x%04x\n", channel);
                l2cap_decline_connection(channel);
                break;
            }
            device->hid_interrupt_cid = channel;
            l2cap_accept_connection(channel);
            break;
        default:
            printf("Unknown PSM = 0x%02x\n", psm);
    }
}

static int my_hid_device_channel_opened(uint8_t* packet, uint16_t channel) {
    uint16_t psm;
    uint8_t status;
    uint16_t local_cid;
    uint16_t remote_cid;
    hci_con_handle_t handle;
    bd_addr_t address;
    my_hid_device_t* device;
    uint8_t incoming;

    printf("L2CAP_EVENT_CHANNEL_OPENED (channel=0x%04x)\n", channel);
    status = l2cap_event_channel_opened_get_status(packet);
    if (status){
        printf("L2CAP Connection failed: 0x%02x\n", status);
        return -1;
    }
    psm = l2cap_event_channel_opened_get_psm(packet);
    local_cid = l2cap_event_channel_opened_get_local_cid(packet);
    remote_cid = l2cap_event_channel_opened_get_remote_cid(packet);
    handle = l2cap_event_channel_opened_get_handle(packet);
    incoming = l2cap_event_channel_opened_get_incoming(packet);
    l2cap_event_channel_opened_get_address(packet, address);
    printf("PSM: 0x%04x, Local CID=0x%04x, Remote CID=0x%04x, handle=0x%04x, incoming=%d\n", psm, local_cid, remote_cid, handle, incoming);

    device = my_hid_device_get_instance_for_address(address);
    if (device == NULL) {
        printf("could not find device for address\n");
        return -1;
    }

    switch (psm){
        case PSM_HID_CONTROL:
            device->hid_control_cid = l2cap_event_channel_opened_get_local_cid(packet);
            printf("HID Control opened, cid 0x%02x\n", device->hid_control_cid);
            break;
        case PSM_HID_INTERRUPT:
            device->hid_interrupt_cid = l2cap_event_channel_opened_get_local_cid(packet);
            printf("HID Interrupt opened, cid 0x%02x\n", device->hid_interrupt_cid);
            // Don't request HID descriptor if we already have it.
            if (device->hid_descriptor_len == 0) {
                // Needed for the SDP query since it only supports oe SDP query at the time.
                if (current_device != NULL) {
                    printf("Error: Ouch, another SDP query is in progress. Try again later.\n");
                } else {
                    current_device = device;
                    status = sdp_client_query_uuid16(&handle_sdp_client_query_result, device->address, BLUETOOTH_SERVICE_CLASS_HUMAN_INTERFACE_DEVICE_SERVICE);
                    if (status != 0) {
                        current_device = NULL;
                        printf("FAILED to perform sdp query\n");
                    }
                }
            }
            break;
        default:
            break;
    }

    if (!device->incoming) {
        if (local_cid == 0)
            return -1;
        if (local_cid == device->hid_control_cid){
            printf("Creating HID INTERRUPT channel\n");
            status = l2cap_create_channel(packet_handler, device->address, PSM_HID_INTERRUPT, 48, &device->hid_interrupt_cid);
            if (status){
                printf("Connecting to HID Control failed: 0x%02x\n", status);
                return -1;
            }
            printf("---> new hid interrupt psm = 0x%04x\n", device->hid_interrupt_cid);
        }
        if (local_cid == device->hid_interrupt_cid){
            printf("HID Connection established\n");
        }
    }
    return 0;
}

static void my_hid_device_channel_closed(uint8_t* packet, uint16_t channel) {
    uint16_t local_cid;
    my_hid_device_t* device;

    local_cid = l2cap_event_channel_closed_get_local_cid(packet);
    printf("L2CAP_EVENT_CHANNEL_CLOSED: 0x%04x (channel=0x%04x)\n", local_cid, channel);
    device = my_hid_device_get_instance_for_cid(local_cid);
    if (device == NULL) {
        // Device might already been closed if the Control or Interrupt PSM was closed first.
        printf("INFO: couldn't not find hid_device for cid = 0x%04x\n", local_cid);
        return;
    }
    my_hid_device_set_disconnected(device);
}

static void my_hid_device_remove_entry_with_channel(uint16_t channel) {
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

static void my_hid_device_set_disconnected(my_hid_device_t* device) {
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

static void my_hid_device_assign_joystick_port(my_hid_device_t* device) {
    const uint32_t cod = device->cod;
    const uint32_t minor_cod = cod & MASK_COD_MINOR_MASK;
    if ((cod & MASK_COD_MAJOR_PERIPHERAL) == MASK_COD_MAJOR_PERIPHERAL) {
        if ((minor_cod & MASK_COD_MINOR_POINT_DEVICE) == MASK_COD_MINOR_POINT_DEVICE)
            device->joystick_port = JOYSTICK_PORT_A;
    }
    device->joystick_port = JOYSTICK_PORT_B;
}

int btstack_main(int argc, const char * argv[]){

    (void)argc;
    (void)argv;

    memset(devices, 0, sizeof(devices));

    // gpio init
    gpio_joy_init();

    // Initialize L2CAP
    l2cap_init();

    // hid_device_setup();
    hid_host_setup();

    // btstack_stdin_setup(stdin_process);
    // Turn on the device
    hci_power_control(HCI_POWER_ON);
    return 0;
}

/* EXAMPLE_END */
