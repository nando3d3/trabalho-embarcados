#include <stdio.h>
#include "relay.h"

// #define RELAY_PIN 26

gpio_num_t relay_pin;

void relay_init(gpio_num_t RELAY_PIN){

    relay_pin = RELAY_PIN;
    esp_rom_gpio_pad_select_gpio(relay_pin);
    gpio_set_direction(relay_pin, GPIO_MODE_OUTPUT);
}

void relay_set_state(int state) {
    gpio_set_level(relay_pin, state);
}