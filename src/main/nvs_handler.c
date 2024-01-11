#include <stdio.h>
#include <string.h>
#include "esp_log.h"
#include "freertos/FreeRTOS.h"

#include "nvs.h"
#include "nvs_flash.h"
#include "nvs_handler.h"

#define TAG "NVS"

void read_nvs(char* variable_name, void* value, size_t size)
{
  ESP_ERROR_CHECK(nvs_flash_init_partition("DATA"));
  nvs_handle partition_handler;
  esp_err_t res = nvs_open_from_partition("DATA", "save", NVS_READONLY, &partition_handler);

  if(res == ESP_ERR_NVS_NOT_FOUND)
    ESP_LOGE(TAG, "Namespace: save não encontrado.");
  else {
    res = nvs_get_blob(partition_handler, variable_name, value, &size);
    if(res != ESP_OK)
      ESP_LOGE(TAG, "Não foi possível ler '%s' no NVS (%s)", variable_name,  esp_err_to_name(res));
    nvs_close(partition_handler);
  }
}

void write_nvs(char* variable_name, void* value, size_t size)
{
  ESP_ERROR_CHECK(nvs_flash_init_partition("DATA"));
  nvs_handle partition_handler;
  esp_err_t res = nvs_open_from_partition("DATA", "save", NVS_READWRITE, &partition_handler);

  if(res == ESP_ERR_NVS_NOT_FOUND)
    ESP_LOGE(TAG, "Namespace: save, não encontrado");
  else {
    res = nvs_set_blob(partition_handler, variable_name, value, size);
    if(res != ESP_OK)
      ESP_LOGE(TAG, "Não foi possível escrever '%s' no NVS (%s)", variable_name,  esp_err_to_name(res));

    nvs_commit(partition_handler);
    nvs_close(partition_handler);
  }
}