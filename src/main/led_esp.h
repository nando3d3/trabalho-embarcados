#ifndef LED_ESP_H
#define LED_ESP_H

#include "driver/ledc.h"

// #define LED 2
#define LEDC_CHANNEL_0 0

// Função para inicializar o LED e o PWM com uma frequência padrão
void led_init(gpio_num_t LED);

// Função para alterar a frequência do PWM em tempo de execução
void led_set_frequency(uint32_t freq);

#endif /* LED_CONTROL_H_ */