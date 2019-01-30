/****************************************************************************
http://retro.moe/unijoysticle

Copyright 2017 Ricardo Quesada

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

// ESP32 version

#include "gpio_joy.h"

#include "driver/gpio.h"

// GPIO map for MH-ET Live mini-kit board.
// Same GPIOs as Wemos D1 mini (used in Unijoysticle v0.4)
enum {
    GPIO_JOY_A_UP       = GPIO_NUM_26,      // D0
    GPIO_JOY_A_DOWN     = GPIO_NUM_22,      // D1
    GPIO_JOY_A_LEFT     = GPIO_NUM_21,      // D2
    GPIO_JOY_A_RIGHT    = GPIO_NUM_17,      // D3
    GPIO_JOY_A_FIRE     = GPIO_NUM_16,      // D4

    GPIO_JOY_B_UP       = GPIO_NUM_18,      // D5
    GPIO_JOY_B_DOWN     = GPIO_NUM_19,      // D6
    GPIO_JOY_B_LEFT     = GPIO_NUM_23,      // D7
    GPIO_JOY_B_RIGHT    = GPIO_NUM_5,       // D8
    // GPIO_NUM_3 is assigned to the UART. And although it is possible to
    // rewire the GPIOs for the UART in software, the devkits expects that GPIOS 1 and 3
    // are assigned to UART 0. And I cannot use it.
    // Using GPIO 27 instead, which is the one that is closer to GPIO 3.
    GPIO_JOY_B_FIRE     = GPIO_NUM_27,      // RX
};

static gpio_num_t JOY_A_PORTS[] = {GPIO_JOY_A_UP, GPIO_JOY_A_DOWN, GPIO_JOY_A_LEFT, GPIO_JOY_A_RIGHT, GPIO_JOY_A_FIRE};
static gpio_num_t JOY_B_PORTS[] = {GPIO_JOY_B_UP, GPIO_JOY_B_DOWN, GPIO_JOY_B_LEFT, GPIO_JOY_B_RIGHT, GPIO_JOY_B_FIRE};

static void gpio_joy_update_port(joystick_t* joy, gpio_num_t* gpios);

void gpio_joy_init(void) {
    printf("gpio_joy_init()\n");
    gpio_config_t io_conf;
    io_conf.intr_type = GPIO_INTR_DISABLE;
    io_conf.mode = GPIO_MODE_OUTPUT;
    io_conf.pull_down_en = GPIO_PULLDOWN_DISABLE;
    io_conf.pull_up_en = GPIO_PULLUP_DISABLE;
    // Port A.
    io_conf.pin_bit_mask = (
        (1ULL << GPIO_JOY_A_UP) | (1ULL << GPIO_JOY_A_DOWN) |
        (1ULL << GPIO_JOY_A_LEFT) | (1ULL << GPIO_JOY_A_RIGHT) |
        (1ULL << GPIO_JOY_A_FIRE)
        );
    // Port B.
    io_conf.pin_bit_mask |= (
        (1ULL << GPIO_JOY_B_UP) | (1ULL << GPIO_JOY_B_DOWN) |
        (1ULL << GPIO_JOY_B_LEFT) | (1ULL << GPIO_JOY_B_RIGHT) |
        (1ULL << GPIO_JOY_B_FIRE)
        );

    ESP_ERROR_CHECK( gpio_config(&io_conf) );

    // Set low all GPIOs... just in case.
    const int MAX_GPIOS = sizeof(JOY_A_PORTS)/sizeof(JOY_A_PORTS[0]);
    for (int i=0; i<MAX_GPIOS; i++) {
        ESP_ERROR_CHECK( gpio_set_level(JOY_A_PORTS[i], 0));
        ESP_ERROR_CHECK( gpio_set_level(JOY_B_PORTS[i], 0));
    }
}

void gpio_joy_update_port_a(joystick_t* joy) {
    gpio_joy_update_port(joy, JOY_A_PORTS);
}

void gpio_joy_update_port_b(joystick_t* joy) {
    gpio_joy_update_port(joy, JOY_B_PORTS);
}

static void gpio_joy_update_port(joystick_t* joy, gpio_num_t* gpios) {
    gpio_set_level(gpios[0], !!joy->up);
    gpio_set_level(gpios[1], !!joy->down);
    gpio_set_level(gpios[2], !!joy->left);
    gpio_set_level(gpios[3], !!joy->right);
    gpio_set_level(gpios[4], !!joy->fire);
}