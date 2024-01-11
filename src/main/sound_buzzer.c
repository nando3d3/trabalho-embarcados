#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "driver/gpio.h"
#include "sound_buzzer.h"


// Sensor de som

gpio_num_t sensor_pin;

void sound_sensor_init(gpio_num_t SOUND_SENSOR_PIN)
{   
    sensor_pin = SOUND_SENSOR_PIN;
    // Configurar o pino GPIO do sensor de som
    esp_rom_gpio_pad_select_gpio(sensor_pin); 
    gpio_set_direction(sensor_pin, GPIO_MODE_INPUT);
}

int get_sound_sensor()
{
    int sound_level = gpio_get_level(sensor_pin);
    return sound_level;
}
// -----------------------------------------------------------------

// Buzzer

gpio_num_t buzzer_pin;

void buzzer_init(gpio_num_t BUZZER_PIN){
    // Configurar o pino GPIO do buzzer
    buzzer_pin = BUZZER_PIN;
    esp_rom_gpio_pad_select_gpio(buzzer_pin);
    gpio_set_direction(buzzer_pin, GPIO_MODE_OUTPUT);
}

void buzzer_on_off(int on)
{
    if(on == 1){
        gpio_set_level(buzzer_pin, 1);
    }else{
        gpio_set_level(buzzer_pin, 0);
    }
}