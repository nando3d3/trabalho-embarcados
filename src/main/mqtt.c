#include <stdio.h>
#include <stdint.h>
#include <stddef.h>
#include <string.h>
#include "esp_system.h"
#include "esp_event.h"
#include "esp_netif.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/semphr.h"
#include "freertos/queue.h"
#include "cJSON.h"
#include "lwip/sockets.h"
#include "lwip/dns.h"
#include "lwip/netdb.h"

#include "esp_log.h"
#include "mqtt_client.h"

#include "mqtt.h"

#define TAG "MQTT"

extern SemaphoreHandle_t conexaoMQTTSemaphore;
esp_mqtt_client_handle_t client;

static void log_error_if_nonzero(const char *message, int error_code)
{
    if (error_code != 0) {
        ESP_LOGE(TAG, "Last error %s: 0x%x", message, error_code);
    }
}

float thermostat_temp = 36.0f;
int thermostat_state = 2;

void mqtt_data_handler(const char *data) {
    
    float valor_params = 25.0f;

    cJSON *json = cJSON_Parse(data);
    if (json == NULL) {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL) {
            fprintf(stderr, "Erro no JSON: %s\n", error_ptr);
        }
    }

    cJSON *method = cJSON_GetObjectItemCaseSensitive(json, "method");
    if (cJSON_IsString(method) && (method->valuestring != NULL)) {
        if (strcmp(method->valuestring, "setThermoValue") == 0) {
            cJSON *params = cJSON_GetObjectItemCaseSensitive(json, "params");
            if (cJSON_IsNumber(params)) {
                valor_params = (float)params->valuedouble;
                thermostat_temp = valor_params; // Atribui o valor de params a thermostat_temp
            }
        }
        else if(strcmp(method->valuestring, "setThermoState") == 0){
            cJSON *params = cJSON_GetObjectItemCaseSensitive(json, "params");
            if (cJSON_IsBool(params)) {
                int state = cJSON_IsTrue(params) ? 1 : 0;
                thermostat_state = state; // Define thermostat_state como 1 se params for verdadeiro, senão, define como 0
            }
        }
    }
    printf("ThermoTemp: %f\nThermoState: %d", thermostat_temp, thermostat_state);
    cJSON_Delete(json);
}

static void mqtt_event_handler(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
    ESP_LOGD(TAG, "Event dispatched from event loop base=%s, event_id=%d", base, (int) event_id);
    esp_mqtt_event_handle_t event = event_data;
    esp_mqtt_client_handle_t client = event->client;
    int msg_id;
    switch ((esp_mqtt_event_id_t)event_id) {
    case MQTT_EVENT_CONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_CONNECTED");
        xSemaphoreGive(conexaoMQTTSemaphore);
        msg_id = esp_mqtt_client_subscribe(client, "v1/devices/me/rpc/request/+", 0);
        break;
    case MQTT_EVENT_DISCONNECTED:
        ESP_LOGI(TAG, "MQTT_EVENT_DISCONNECTED");
        break;

    case MQTT_EVENT_SUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_SUBSCRIBED, msg_id=%d", event->msg_id);
        msg_id = esp_mqtt_client_publish(client, "/topic/qos0", "data", 0, 0, 0);
        ESP_LOGI(TAG, "sent publish successful, msg_id=%d", msg_id);
        break;
    case MQTT_EVENT_UNSUBSCRIBED:
        ESP_LOGI(TAG, "MQTT_EVENT_UNSUBSCRIBED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_PUBLISHED:
        ESP_LOGI(TAG, "MQTT_EVENT_PUBLISHED, msg_id=%d", event->msg_id);
        break;
    case MQTT_EVENT_DATA:
        ESP_LOGI(TAG, "MQTT_EVENT_DATA");
        printf("TOPIC=%.*s\r\n", event->topic_len, event->topic);
        printf("DATA=%.*s\r\n", event->data_len, event->data);
        mqtt_data_handler(event->data);
        break;
    case MQTT_EVENT_ERROR:
        ESP_LOGI(TAG, "MQTT_EVENT_ERROR");
        if (event->error_handle->error_type == MQTT_ERROR_TYPE_TCP_TRANSPORT) {
            log_error_if_nonzero("reported from esp-tls", event->error_handle->esp_tls_last_esp_err);
            log_error_if_nonzero("reported from tls stack", event->error_handle->esp_tls_stack_err);
            log_error_if_nonzero("captured as transport's socket errno",  event->error_handle->esp_transport_sock_errno);
            ESP_LOGI(TAG, "Last errno string (%s)", strerror(event->error_handle->esp_transport_sock_errno));

        }
        break;
    default:
        ESP_LOGI(TAG, "Other event id:%d", event->event_id);
        break;
    }
}

void mqtt_start()
{
    esp_mqtt_client_config_t mqtt_config = {
        // .broker.address.uri = "mqtt://test.mosquitto.org",
        .broker.address.uri = "mqtt://164.41.98.25/",
        .credentials.username = "52qv2r4uFe0wza2SxbBP"

    };
    client = esp_mqtt_client_init(&mqtt_config);
    esp_mqtt_client_register_event(client, ESP_EVENT_ANY_ID, mqtt_event_handler, NULL);
    esp_mqtt_client_start(client);
}

void mqtt_envia_mensagem(char * topico, char * mensagem)
{
    int message_id = esp_mqtt_client_publish(client, topico, mensagem, 0, 1, 0);
    ESP_LOGI(TAG, "Mesnagem enviada, ID: %d", message_id);
}
