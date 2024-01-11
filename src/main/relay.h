#ifndef RELAY_H
#define RELAY_H

#include "driver/gpio.h"

// #define RELAY_PIN 26

void relay_init(gpio_num_t RELAY_PIN);
void relay_set_state(int state);

#endif /* RELAY_H */
