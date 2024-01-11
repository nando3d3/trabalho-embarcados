#ifndef SOUND_BUZZER_H
#define SOUND_BUZZER_H

#include "driver/gpio.h"

// #define SOUND_SENSOR_PIN GPIO_NUM_2
// #define BUZZER_PIN GPIO_NUM_13 // Altere este valor para o pino desejado para o buzzer

void sound_sensor_init(gpio_num_t SOUND_SENSOR_PIN);
int get_sound_sensor();
void buzzer_init(gpio_num_t BUZZER_PIN);
void buzzer_on_off(int on);

#endif /* SOUND_BUZZER_H */
