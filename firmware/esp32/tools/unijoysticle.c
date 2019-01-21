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
 * Unijoysticle code:
 * Code based on BlueKitchen examples / tests. In particular:
 *   - hid_host_test.c
 *   - hid_device.c
 *   - gap_inquire.c
 *   - hid_device_test.c
 */

#define __BTSTACK_FILE__ "hid_host_demo.c"

/*
 * hid_host_demo.c
 */

/* EXAMPLE_START(hid_host_demo): HID Host Demo
 *
 * @text This example implements an HID Host. For now, it connnects to a fixed device, queries the HID SDP
 * record and opens the HID Control + Interrupt channels
 */

#include <inttypes.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "btstack_config.h"
#include "btstack.h"

#define INQUIRY_INTERVAL          5
#define MASK_COD_MAJOR_PERIPHERAL 0x0500   // 0b0000_0101_0000_0000
#define MASK_COD_MINOR_ALL        0x003c   //             0011_1100
#define MASK_COD_MINOR_GAMEPAD    0x0008   //             0000_1000
#define MASK_COD_MINOR_JOYSTICK   0x0004   //             0000_0100
#define MAX_ATTRIBUTE_VALUE_SIZE  512      // Apparently PS4 has a 470-bytes report
#define MAX_DEVICES               4
#define MTU                       100

// SDP
static uint8_t            attribute_value[MAX_ATTRIBUTE_VALUE_SIZE];
static const unsigned int attribute_value_buffer_size = MAX_ATTRIBUTE_VALUE_SIZE;

enum DEVICE_STATE { REMOTE_NAME_REQUEST, REMOTE_NAME_INQUIRED, REMOTE_NAME_FETCHED };
typedef struct  {
    bd_addr_t           address;
    hci_con_handle_t    con_handle;
    uint8_t             page_scan_repetition_mode;
    uint16_t            clock_offset;
    uint32_t            cod;

    // state
    uint8_t             incoming;
    uint8_t             connected;

    // SDP
    uint8_t             hid_descriptor[MAX_ATTRIBUTE_VALUE_SIZE];
    uint16_t            hid_descriptor_len;

    // Channels
    uint16_t            hid_control_psm;
    uint16_t            hid_interrupt_psm;
    uint16_t            remote_hid_control_psm;         // must be PSM_HID_CONTROL
    uint16_t            remote_hid_interrupt_psm;       // must be PSM_HID_INTERRUPT
    enum DEVICE_STATE  state;
} my_hid_device_t;

my_hid_device_t devices[MAX_DEVICES];
int deviceCount = 0;

// Asus
// static const char * remote_addr_string = "54:A0:50:CD:A6:2F";
// static bd_addr_t remote_addr;

static btstack_packet_callback_registration_t hci_event_callback_registration;

// Needed for queries
    typedef enum {
    HID_HOST_IDLE,              // host is doing nothing
    HID_HOST_SCAN,              // host is scanning
    HID_HOST_CONNECTED,         // host has a device connected
} hid_host_state_t;

static hid_host_state_t hid_host_state = HID_HOST_IDLE;

static void packet_handler(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void handle_sdp_client_query_result(uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size);
static void print_gamepad(void);
static void hid_host_handle_interrupt_report(const uint8_t * report, uint16_t report_len);
static int getDeviceIndexForAddress(bd_addr_t addr);
static void continue_remote_names(void);
static void start_scan(void);
static int is_device_gamepad(uint32_t cod);
static my_hid_device_t* my_hid_device_get_instance_for_cid(uint16_t cid);
void my_hid_device_disconnect(my_hid_device_t* device);
int btstack_main(int argc, const char * argv[]);

static void hid_host_setup(void){

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
    uint8_t         status;

    // printf_hexdump(packet, size);

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
                                        de_element_get_uint16(des_iterator_get_element(&prot_it), &devices[0].remote_hid_control_psm);
                                        printf("SDP HID Control PSM: 0x%04x\n", (int) devices[0].remote_hid_control_psm);
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
                                            de_element_get_uint16(des_iterator_get_element(&prot_it), &devices[0].remote_hid_interrupt_psm);
                                            printf("SDP HID Interrupt PSM: 0x%04x\n", (int) devices[0].remote_hid_interrupt_psm);
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
                                    devices[0].hid_descriptor_len = de_get_data_size(element);
                                    printf("SDP HID Descriptor (%d):\n", devices[0].hid_descriptor_len);
                                    memcpy(devices[0].hid_descriptor, descriptor, devices[0].hid_descriptor_len);
                                    printf_hexdump(devices[0].hid_descriptor, devices[0].hid_descriptor_len);
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
            if (devices[0].remote_hid_control_psm != PSM_HID_CONTROL) {
                printf("Invalid Control PSM missing. Expecting = 0x%04x, got = 0x%04x\n", PSM_HID_CONTROL, devices[0].remote_hid_control_psm);
                break;
            }
            if (devices[0].remote_hid_interrupt_psm != PSM_HID_INTERRUPT) {
                printf("Invalid Control PSM missing. Expecting = 0x%04x, got = 0x%04x\n", PSM_HID_INTERRUPT, devices[0].remote_hid_interrupt_psm);
                break;
            }
            printf("Setup HID\n");

            if (devices[0].incoming == 0 && hid_host_state != HID_HOST_CONNECTED) {
                printf("Creating HID CONTROL channel\n");
                status = l2cap_create_channel(packet_handler, devices[0].address, PSM_HID_CONTROL, 48, &devices[0].hid_control_psm);
                if (status){
                    printf("Connecting to HID Control failed: 0x%02x\n", status);
                } else {
                    hid_host_state = HID_HOST_CONNECTED;
                    printf("--> new control psm = 0x%04x\n", devices[0].hid_control_psm);
                }
            }
            break;
    }
}

static void packet_handler (uint8_t packet_type, uint16_t channel, uint8_t *packet, uint16_t size)
{
    uint8_t   event;
    bd_addr_t event_addr;
    uint8_t   status;
    uint16_t  l2cap_cid;
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
                        // sdp_client_query_uuid16(&handle_sdp_client_query_result, remote_addr, BLUETOOTH_SERVICE_CLASS_HUMAN_INTERFACE_DEVICE_SERVICE);
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
                {
                    uint16_t psm = l2cap_event_incoming_connection_get_psm(packet);
                    printf("L2CAP_EVENT_INCOMING_CONNECTION. PSM = 0x%04x\n", psm);
                    switch (psm) {
                        case PSM_HID_CONTROL:
                        case PSM_HID_INTERRUPT:
                            l2cap_accept_connection(channel);
                            l2cap_event_incoming_connection_get_address(packet, devices[0].address); 
                            devices[0].con_handle = l2cap_event_incoming_connection_get_handle(packet);
                            devices[0].incoming = 1;
                            break;
                            // FIXME: we decline connection, and we connect to them
                            // l2cap_decline_connection(channel);
                            // l2cap_accept_connection(channel);
                            // l2cap_event_incoming_connection_get_address(packet, remote_addr); 
                            // hid_host_state = HID_HOST_CONNECTED;
                            // sdp_client_query_uuid16(&handle_sdp_client_query_result, remote_addr, BLUETOOTH_SERVICE_CLASS_HUMAN_INTERFACE_DEVICE_SERVICE);
                            break;
                        default:
                            printf("Unknown PSM = 0x%02x\n", psm);
                    }
                    break;
                }
                case L2CAP_EVENT_CHANNEL_OPENED: 
                {
                    printf("L2CAP_EVENT_CHANNEL_OPENED\n");
                    status = packet[2];
                    if (status){
                        printf("L2CAP Connection failed: 0x%02x\n", status);
                        break;
                    }
                    uint16_t psm = l2cap_event_channel_opened_get_psm(packet);
                    printf("PSM: 0x%04x\n", psm);
                    switch (psm){
                        case PSM_HID_CONTROL:
                            devices[0].hid_control_psm = l2cap_event_channel_opened_get_local_cid(packet);
                            printf("HID Control opened, cid 0x%02x\n", devices[0].hid_control_psm);
                            break;
                        case PSM_HID_INTERRUPT:
                            devices[0].hid_interrupt_psm = l2cap_event_channel_opened_get_local_cid(packet);
                            printf("HID Interrupt opened, cid 0x%02x\n", devices[0].hid_interrupt_psm);
                            status = sdp_client_query_uuid16(&handle_sdp_client_query_result, devices[0].address, BLUETOOTH_SERVICE_CLASS_HUMAN_INTERFACE_DEVICE_SERVICE);
                            if (status != 0) {
                                printf("FAILED to perform sdp query\n");
                            }
                            break;
                        default:
                            break;
                    }
                    if (devices[0].incoming == 0) {
                        l2cap_cid = little_endian_read_16(packet, 13);
                        if (!l2cap_cid) break;
                        if (l2cap_cid == devices[0].hid_control_psm){
                            printf("Creating HID INTERRUPT channel\n");
                            status = l2cap_create_channel(packet_handler, devices[0].address, PSM_HID_INTERRUPT, 48, &devices[0].hid_interrupt_psm);
                            if (status){
                                printf("Connecting to HID Control failed: 0x%02x\n", status);
                                break;
                            }
                            printf("---> new hid interrupt psm = 0x%04x\n", devices[0].hid_interrupt_psm); 
                        }                        
                        if (l2cap_cid == devices[0].hid_interrupt_psm){
                            printf("HID Connection established\n");
                            hid_host_state = HID_HOST_CONNECTED;
                        }
                    }
                    break;
                }
                case L2CAP_EVENT_CHANNEL_CLOSED:
                    printf("L2CAP_EVENT_CHANNEL_CLOSED\n");
                    l2cap_cid = l2cap_event_channel_closed_get_local_cid(packet);
                    device = my_hid_device_get_instance_for_cid(l2cap_cid);
                    if (device == NULL) {
                        printf("ERROR: couldn't not find hid_device for cid = 0x%04x\n", l2cap_cid);
                    }
                    my_hid_device_disconnect(device);
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

                    if (is_device_gamepad(cod)) {
                        if (deviceCount >= MAX_DEVICES) break;  // already full
                        index = getDeviceIndexForAddress(event_addr);
                        if (index >= 0) break;   // already in our list

                        memcpy(devices[deviceCount].address, event_addr, 6);
                        devices[deviceCount].page_scan_repetition_mode = page_scan_repetition_mode;
                        devices[deviceCount].clock_offset = clock_offset;
                        devices[deviceCount].cod = cod;
                        if (gap_event_inquiry_result_get_name_available(packet)){
                            char name_buffer[240];
                            int name_len = gap_event_inquiry_result_get_name_len(packet);
                            memcpy(name_buffer, gap_event_inquiry_result_get_name(packet), name_len);
                            name_buffer[name_len] = 0;
                            printf(", name '%s'", name_buffer);
                            devices[deviceCount].state = REMOTE_NAME_FETCHED;;
                        } else {
                            devices[deviceCount].state = REMOTE_NAME_REQUEST;
                        }
                        deviceCount++;
                        memcpy(devices[0].address, event_addr, 6);
                        sdp_client_query_uuid16(&handle_sdp_client_query_result, devices[0].address, BLUETOOTH_SERVICE_CLASS_HUMAN_INTERFACE_DEVICE_SERVICE);

                    }
                    printf("\n");
                    break;
                case GAP_EVENT_INQUIRY_COMPLETE:
                    for (int i=0;i<deviceCount;i++) {
                        // retry remote name request
                        if (devices[i].state == REMOTE_NAME_INQUIRED) {
                            devices[i].state = REMOTE_NAME_REQUEST;
                        }
                    }
                    continue_remote_names();
                    break;

                case HCI_EVENT_REMOTE_NAME_REQUEST_COMPLETE:
                    reverse_bd_addr(&packet[3], event_addr);
                    index = getDeviceIndexForAddress(event_addr);
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
            if (channel == devices[0].hid_interrupt_psm){
                printf("HID Interrupt Packet: ");
                printf_hexdump(packet, size);
                hid_host_handle_interrupt_report(packet,  size);
                print_gamepad();
            } else if (channel == devices[0].hid_control_psm){
                printf("\nHID Control: ");
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

static int is_device_gamepad(uint32_t cod) {
    if ((cod & MASK_COD_MAJOR_PERIPHERAL) == MASK_COD_MAJOR_PERIPHERAL) {
        // device is a peripheral: keyboard, mouse, joystick, gamepad...
        // but we only care about joysticks and gamepads
        uint32_t minor_cod = cod & MASK_COD_MINOR_ALL;
        return (minor_cod == MASK_COD_MINOR_GAMEPAD || minor_cod == MASK_COD_MINOR_JOYSTICK);
    }
    return 0;
}

static int getDeviceIndexForAddress( bd_addr_t addr){
    int j;
    for (j=0; j< deviceCount; j++){
        if (bd_addr_cmp(addr, devices[j].address) == 0){
            return j;
        }
    }
    return -1;
}

static int has_more_remote_name_requests(void){
    int i;
    for (i=0;i<deviceCount;i++) {
        if (devices[i].state == REMOTE_NAME_REQUEST) return 1;
    }
    return 0;
}

static void do_next_remote_name_request(void){
    int i;
    for (i=0;i<deviceCount;i++) {
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
    DPAD_UP = 1 << 0,
    DPAD_DOWN = 1 << 1,
    DPAD_RIGHT = 1 << 2,
    DPAD_LEFT = 1 << 3,
};

enum {
    MISC_AC_HOME = 1 << 0,
    MISC_AC_BACK = 1 << 1,
    MISC_SYS_MAIN_MENU = 1 << 2,
};

typedef struct gamepad {
    // Usage Page: 0x01 (Generic Desktop Controls)
    uint8_t hat;
    int16_t x;
    int16_t y;
    int16_t z;
    int16_t rx;
    int16_t ry;
    int16_t rz;
    uint8_t dpad;

    // Usage Page: 0x02 (Sim controls)
    int32_t     brake;
    int32_t     accelerator;

    // Usage Page: 0x06 (Generic dev controls)
    uint16_t    battery;

    // Usage Page: 0x08 (LED)
    uint8_t     leds;

    // Usage Page: 0x09 (Button)
    uint32_t    buttons;

    // Misc buttos (from 0x0c (Consumer) and others)
    uint8_t    misc_buttons;
} gamepad_t;

static gamepad_t g_gamepad;

static void print_gamepad(void) {
    printf("x=%d, y=%d, z=%d, rx=%d, ry=%d, rz=%d, hat=0x%02x, dpad=0x%02x, accel=%d, brake=%d, buttons=0x%08x, misc=0x%02x, leds=0x%02x\n",
            g_gamepad.x, g_gamepad.y, g_gamepad.z,
            g_gamepad.rx, g_gamepad.ry, g_gamepad.rz,
            g_gamepad.hat,
            g_gamepad.dpad,
            g_gamepad.accelerator, g_gamepad.brake,
            g_gamepad.buttons,
            g_gamepad.misc_buttons,
            g_gamepad.leds
          );

    // gpio_set_level(GPIO_NUM_23, g_gamepad.buttons[1] != 0);
}

static void hid_host_handle_interrupt_report(const uint8_t * report, uint16_t report_len) {
    // check if HID Input Report
    if (report_len < 1) return;
    if (*report != 0xa1) return;
    report++;
    report_len--;
    btstack_hid_parser_t parser;
    btstack_hid_parser_init(&parser, devices[0].hid_descriptor, devices[0].hid_descriptor_len, HID_REPORT_TYPE_INPUT, report, report_len);
    while (btstack_hid_parser_has_more(&parser)){
        uint16_t usage_page;
        uint16_t usage;
        int32_t  value;
        btstack_hid_parser_get_field(&parser, &usage_page, &usage, &value);


        /*
        printf("usage_page = 0x%04x, usage = 0x%04x, value = 0x%x - ", usage_page, usage, value);
        printf("min=%d, max=%d, lmin=%d, lmax=%d\n", parser.usage_minimum, parser.usage_maximum, parser.global_logical_minimum, parser.global_logical_maximum);
        */

        switch (usage_page) {
            case 0x01:  // Generic Desktop controls
                switch (usage) {
                    case 0x30:  // x
                        g_gamepad.x = value; //- (parser.global_logical_maximum/2);
                        break;
                    case 0x31:  // y
                        g_gamepad.y = value; //- (parser.global_logical_maximum/2);
                        break;
                    case 0x32:  // z
                        g_gamepad.z = value; // - (parser.global_logical_maximum/2);
                        break;
                    case 0x33:  // rx
                        g_gamepad.rx = value; // - (parser.global_logical_maximum/2);
                        break;
                    case 0x34:  // ry
                        g_gamepad.ry = value; // - (parser.global_logical_maximum/2);
                        break;
                    case 0x35:  // rz
                        g_gamepad.rz = value; // - (parser.global_logical_maximum/2);
                        break;
                    case 0x39:  // switch hat
                        g_gamepad.hat = value;
                        break;
                    case 0x85: // system main menu
                        if (value)
                            g_gamepad.misc_buttons |= MISC_SYS_MAIN_MENU;
                        else
                            g_gamepad.misc_buttons &= ~MISC_SYS_MAIN_MENU;
                        break;
                    case 0x90:  // dpad up
                        if (value)
                            g_gamepad.dpad |= DPAD_UP;
                        else
                            g_gamepad.dpad &= ~DPAD_UP;
                        break;
                    case 0x91:  // dpad down
                        if (value)
                            g_gamepad.dpad |= DPAD_DOWN;
                        else
                            g_gamepad.dpad &= ~DPAD_DOWN;
                        break;
                    case 0x92:  // dpad right
                        if (value)
                            g_gamepad.dpad |= DPAD_RIGHT;
                        else
                            g_gamepad.dpad &= ~DPAD_RIGHT;
                        break;
                    case 0x93:  // dpad left
                        if (value)
                            g_gamepad.dpad |= DPAD_LEFT;
                        else
                            g_gamepad.dpad &= ~DPAD_LEFT;
                        break;
                    default:
                        printf("Unsupported usage: 0x%04x for page: 0x%04x. value=0x%x\n", usage, usage_page, value);
                        break;
                }
                break;
            case 0x02:  // Simulation controls
                switch (usage) {
                    case 0xc4:  // accelerator
                        g_gamepad.accelerator = value;
                        break;
                    case 0xc5:  // brake
                        g_gamepad.brake = value;
                        break;
                    default:
                        printf("Unsupported usage: 0x%04x for page: 0x%04x. value=0x%x\n", usage, usage_page, value);
                        break;
                };
                break;
            case 0x06: // Generic Device Controls Page
                switch (usage) {
                    case 0x20: // Battery Strength
                        g_gamepad.battery = value;
                        break;
                    default:
                        printf("Unsupported usage: 0x%04x for page: 0x%04x. value=0x%x\n", usage, usage_page, value);
                        break;
                }
                break;
            case 0x08:  // LEDs
            {
                const uint8_t led_idx = usage - 1;
                if (led_idx < 8) {
                    if (value)
                        g_gamepad.leds |= (1 << led_idx);
                    else
                        g_gamepad.leds &= ~(1 << led_idx);
                } else {
                    printf("Unsupported usage: 0x%04x for page: 0x%04x. value=0x%x\n", usage, usage_page, value);
                }
                break;
            }
            case 0x09:  // Button
            {
                // we start with usage - 1 since "button 0" seems that is not being used
                const uint16_t button_idx = usage-1;
                if (button_idx < 32) {
                    if (value)
                        g_gamepad.buttons |= (1 << button_idx);
                    else
                        g_gamepad.buttons &= ~(1 << button_idx);
                } else {
                    printf("Unsupported usage: 0x%04x for page: 0x%04x. value=0x%x\n", usage, usage_page, value);
                }
                break;
            }
            case 0x0c:  // Consumer
                switch (usage) {
                    case 0x0223:    // home
                        if (value)
                            g_gamepad.misc_buttons |= MISC_AC_HOME;
                        else
                            g_gamepad.misc_buttons &= ~MISC_AC_HOME;
                        break;
                    case 0x0224:    // back
                        if (value)
                            g_gamepad.misc_buttons |= MISC_AC_BACK;
                        else
                            g_gamepad.misc_buttons &= ~MISC_AC_BACK;
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
}

// Hid device related
static my_hid_device_t* my_hid_device_get_instance_for_cid(uint16_t cid){
    if (cid == 0)
        return NULL;

    for (int i=0; i<MAX_DEVICES; i++) {
        if (devices[i].hid_interrupt_psm == cid || devices[i].hid_control_psm == cid) {
            return &devices[i];
        }
    }
    return NULL;
}

void my_hid_device_disconnect(my_hid_device_t* device) {
    device->connected = 0;
    device->incoming = 0;
    device->hid_control_psm = 0;
    device->hid_interrupt_psm = 0;
    device->remote_hid_control_psm = 0;
    device->remote_hid_interrupt_psm = 0;
}

int btstack_main(int argc, const char * argv[]){

    (void)argc;
    (void)argv;
    
    memset(devices, 0, sizeof(devices));

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
