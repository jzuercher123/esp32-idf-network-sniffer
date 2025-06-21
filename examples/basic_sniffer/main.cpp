#include <stdio.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "network_sniffer.h"

static const char *TAG = "BASIC_SNIFFER";

void app_main(void)
{
    ESP_LOGI(TAG, "Basic Network Sniffer Example");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Initialize ESP-NETIF
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    // Create sniffer
    NetworkSniffer sniffer;
    ESP_ERROR_CHECK(sniffer.init());
    
    // Start sniffing on channel 6 (common WiFi channel)
    ESP_LOGI(TAG, "Starting sniffing on channel 6");
    ESP_ERROR_CHECK(sniffer.start_sniffing(6));
    
    // Keep running
    while (1) {
        ESP_LOGI(TAG, "Sniffer active on channel %d", sniffer.get_current_channel());
        vTaskDelay(pdMS_TO_TICKS(10000)); // Log every 10 seconds
    }
} 