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

static const char *TAG = "CHANNEL_HOPPER";

// Channel hopping configuration
#define HOP_INTERVAL_MS 5000  // 5 seconds per channel
#define CHANNEL_COUNT 13      // 2.4GHz channels 1-13

void app_main(void)
{
    ESP_LOGI(TAG, "Channel Hopping Network Sniffer Example");
    
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
    
    // Start on channel 1
    uint8_t current_channel = 1;
    ESP_LOGI(TAG, "Starting channel hopping sniffer");
    ESP_ERROR_CHECK(sniffer.start_sniffing(current_channel));
    
    // Channel hopping loop
    while (1) {
        ESP_LOGI(TAG, "Currently sniffing on channel %d", current_channel);
        
        // Wait for hop interval
        vTaskDelay(pdMS_TO_TICKS(HOP_INTERVAL_MS));
        
        // Move to next channel
        current_channel = (current_channel % CHANNEL_COUNT) + 1;
        
        // Stop current sniffing and start on new channel
        ESP_LOGI(TAG, "Hopping to channel %d", current_channel);
        sniffer.stop_sniffing();
        ESP_ERROR_CHECK(sniffer.start_sniffing(current_channel));
    }
} 