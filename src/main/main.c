//idf.py build
//idf.py -p /dev/ttyUSB0 flash monitor

#include <stdio.h>
#include <string.h>
#include "nvs_flash.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_http_client.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "driver/i2c.h"
#include <math.h>

#include "wifi.h"
#include "mqtt.h"
#include "dht.h"
#include "relay.h"
#include "sound_buzzer.h"
#include "i2c-lcd.h"
#include "led_esp.h"
#include "nvs_handler.h"
#include "dht.h"


static const char *TAG = "SensorDHT";
static const char *TAGdisp = "display-lcd";

#define SENSOR_TYPE DHT_TYPE_DHT11

// Pino de dados do sensor conectado ao GPIO 4 (D4) da placa
#define DHT_PIN GPIO_NUM_4
#define BUZZER_PIN GPIO_NUM_13
#define RELAY_PIN GPIO_NUM_26
#define SOUND_SENSOR_PIN GPIO_NUM_35
#define LED 2

SemaphoreHandle_t conexaoWifiSemaphore;
SemaphoreHandle_t conexaoMQTTSemaphore;

char mensagem[50] = "";
float temperatura, umidade = 30;
int led_state = 0;
char ledStatemsg[50] = "";

void conectadoWifi(void *params) {
    while (true) {
        if (xSemaphoreTake(conexaoWifiSemaphore, portMAX_DELAY)) {
            // Processamento Internet
            mqtt_start();
        }
    }
}

void escreveNVS() {
        int32_t value_int = umidade;
        char* name_var_int = "umidade";

        float value_float = temperatura;
        char* name_var_float = "temperatura";

        write_nvs(name_var_int, (int32_t*) &value_int,(size_t) sizeof(int32_t));
        write_nvs(name_var_float, (float*) &value_float,(size_t) sizeof(float));
}

void leituraDHT(void *params) {
    while (true){
        if (dht_read_float_data(SENSOR_TYPE, DHT_PIN, &umidade, &temperatura) == ESP_OK) {
            sprintf(mensagem, "{\"temperatura1\": %.2f, \"umidade1\": %.2f}", temperatura, umidade);
            //printf("%s", mensagem);
        }
        else {
            ESP_LOGE(TAG, "Erro ao ler dados do sensor DHT11");
        }
        vTaskDelay(3000 / portTICK_PERIOD_MS);        
    }
}

void controlaRelay(void *params) {
    int relayStateAnterior = -1;
    int relayEstadoAtual = -1;

    while (1) {
        switch (thermostat_state) {
            // heat
            case 1:
                if (temperatura < thermostat_temp) {
                    relay_set_state(0);  // desliga o relé se a temperatura estiver abaixo do valor desejado
                } else {
                    relay_set_state(1);  // liga o relé se a temperatura estiver no valor desejado ou acima
                }
                break;
            //cold
            case 0:
                if (temperatura > thermostat_temp) {
                    relay_set_state(0);
                } else {
                    relay_set_state(1);
                }
                break;

            default:
                relay_set_state(1);  // Mantém o relé ligado para qualquer outro estado
                break;
        }

        relayEstadoAtual = (thermostat_state == 1 && temperatura < thermostat_temp) || (thermostat_state == 0 && temperatura > thermostat_temp) ? 0 : 1;

        // verifica se o estado do relé foi alterado de desligado (0) para ligado (1)
        if (relayStateAnterior != relayEstadoAtual) {
            buzzer_on_off(1);  // Liga o buzzer
            vTaskDelay(400 / portTICK_PERIOD_MS); 
            buzzer_on_off(0); 
        }

        relayStateAnterior = relayEstadoAtual;
        vTaskDelay(1000 / portTICK_PERIOD_MS);
    }
}

char buffer[10];
float num = 12.34;

/**
 * @brief i2c master initialization
 */
static esp_err_t i2c_master_init(void)
{
    int i2c_master_port = I2C_NUM_0;

    i2c_config_t conf = {
        .mode = I2C_MODE_MASTER,
        .sda_io_num = GPIO_NUM_21,
        .scl_io_num = GPIO_NUM_22,
        .sda_pullup_en = GPIO_PULLUP_ENABLE,
        .scl_pullup_en = GPIO_PULLUP_ENABLE,
        .master.clk_speed = 100000,
    };

    i2c_param_config(i2c_master_port, &conf);

    return i2c_driver_install(i2c_master_port, conf.mode, 0, 0, 0);
}

void controlaDisplay(void *params) {
    while (true) {
        char tempString[20], modeString[20];
        sprintf(tempString, "TEMP: %.2f C", temperatura);
        

        // atualizar o display com a temperatura
        lcd_clear();
        vTaskDelay(50 / portTICK_PERIOD_MS);
        lcd_put_cur(0, 0);
        lcd_send_string(tempString); 
        lcd_put_cur(1, 0);
        if (thermostat_state == 1){
            sprintf(modeString, "MODO: Heat %.1f C", thermostat_temp);
            lcd_send_string(modeString);
        }
        else if (thermostat_state == 0)
        {
            sprintf(modeString, "MODO: Cold %.1f C", thermostat_temp);
            lcd_send_string(modeString);
        }
        
        vTaskDelay(1000 / portTICK_PERIOD_MS);


    }
}

void freqLED(void *params) {
    while (1) {

        float dif_temperatura = fabs(temperatura - thermostat_temp);

        if (dif_temperatura <= 1.0){

            led_set_frequency(30);
            led_state = 1;
        }
        else{
            led_set_frequency(5);
            led_state = !led_state;
        }
        vTaskDelay(3000 / portTICK_PERIOD_MS);
    }
}

void trataComunicacaoComServidor(void *params) {
    if (xSemaphoreTake(conexaoMQTTSemaphore, portMAX_DELAY)) {

        while (true) {
            //envia estado do led
            sprintf(ledStatemsg, "{\"onboard_led\": %d}", led_state);
            mqtt_envia_mensagem("v1/devices/me/attributes", ledStatemsg);

            if (strcmp(mensagem, "") != 0) {
                // Envia mensagem para o servidor MQTT
                printf("%s ", mensagem);
                mqtt_envia_mensagem("v1/devices/me/telemetry", mensagem);
                // Escreve temperatura e umidade no NVS
                escreveNVS();
            }
            else {
                printf("Mensagem vazia!");
            }
            vTaskDelay(3000 / portTICK_PERIOD_MS);
        }
    }
}

void sound_sensor_task(void *pvParameters) {
    while (1) {
        int sound_level = get_sound_sensor();
        printf("Nível do som: %d\n", sound_level);
        vTaskDelay(pdMS_TO_TICKS(3000));
    }
}

void app_main(void) {
    // Inicializa o NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    ESP_ERROR_CHECK(i2c_master_init());
    ESP_LOGI(TAGdisp, "Display I2C iniciado com sucesso");

    conexaoWifiSemaphore = xSemaphoreCreateBinary();
    conexaoMQTTSemaphore = xSemaphoreCreateBinary();
    wifi_start();
    relay_init(RELAY_PIN);
    lcd_init();
    lcd_clear();
    buzzer_init(BUZZER_PIN);
    led_init(LED);
    sound_sensor_init(SOUND_SENSOR_PIN);

    xTaskCreate(&conectadoWifi, "Conexão ao MQTT", 4096, NULL, 1, NULL);
    xTaskCreate(&trataComunicacaoComServidor, "Comunicação com Broker", 4096, NULL, 1, NULL);
    xTaskCreate(&leituraDHT, "Leitura do Sensor", 4096, NULL, 1, NULL);
    xTaskCreate(&controlaRelay, "Controle do Relé", 4096, NULL, 1, NULL);
    xTaskCreate(&controlaDisplay, "Atualiza Display", 4096, NULL, 1, NULL);
    xTaskCreate(&freqLED, "Altera freq. led", 2048, NULL, 5, NULL);
    xTaskCreate(&sound_sensor_task, "sound_sensor_task", 2048, NULL, 5, NULL);
}