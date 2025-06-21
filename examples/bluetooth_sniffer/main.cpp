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
#include "bluetooth_comm.h"

static const char *TAG = "BLUETOOTH_SNIFFER";

// Global instances
NetworkSniffer* g_sniffer = nullptr;
BluetoothComm* g_bluetooth = nullptr;

// Packet counter
static uint32_t packet_count = 0;

// Enhanced packet handler that sends data via Bluetooth
void bluetooth_packet_handler(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT && type != WIFI_PKT_DATA) {
        return;
    }
    
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    packet_count++;
    
    // Log packet information
    ESP_LOGI(TAG, "Packet #%lu - Type: %d, Length: %d, Channel: %d, RSSI: %d",
             packet_count, type, pkt->rx_ctrl.sig_len, pkt->rx_ctrl.channel, pkt->rx_ctrl.rssi);
    
    // Send packet info via Bluetooth if connected
    if (g_bluetooth && g_bluetooth->is_connected()) {
        esp_err_t ret = g_bluetooth->send_packet_info(
            pkt->rx_ctrl.channel,
            pkt->rx_ctrl.rssi,
            pkt->rx_ctrl.sig_len,
            type
        );
        
        if (ret != ESP_OK) {
            ESP_LOGW(TAG, "Failed to send packet info via Bluetooth: %s", esp_err_to_name(ret));
        } else {
            ESP_LOGD(TAG, "Packet info sent via Bluetooth");
        }
    }
    
    // Send packet data (first 20 bytes) if connected
    if (g_bluetooth && g_bluetooth->is_connected() && pkt->rx_ctrl.sig_len > 0) {
        size_t data_len = (pkt->rx_ctrl.sig_len > 20) ? 20 : pkt->rx_ctrl.sig_len;
        g_bluetooth->send_data(pkt->payload, data_len);
    }
}

// Status reporting task
void status_task(void* parameter) {
    while (1) {
        if (g_bluetooth && g_bluetooth->is_connected()) {
            // Send status message
            char status_msg[128];
            snprintf(status_msg, sizeof(status_msg), 
                    "STATUS: Packets=%lu, Channel=%d, Connected=Yes",
                    packet_count, g_sniffer ? g_sniffer->get_current_channel() : 0);
            
            g_bluetooth->send_data((uint8_t*)status_msg, strlen(status_msg));
        }
        
        vTaskDelay(pdMS_TO_TICKS(10000)); // Send status every 10 seconds
    }
}

void app_main(void)
{
    ESP_LOGI(TAG, "Bluetooth Network Sniffer Example");
    
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

    // Initialize Bluetooth communication
    g_bluetooth = new BluetoothComm();
    ESP_ERROR_CHECK(g_bluetooth->init());
    g_bluetooth->set_device_name("ESP32_Sniffer_BT");
    ESP_ERROR_CHECK(g_bluetooth->start_advertising());
    ESP_LOGI(TAG, "Bluetooth advertising started - Look for 'ESP32_Sniffer_BT'");

    // Create and initialize network sniffer
    g_sniffer = new NetworkSniffer();
    ESP_ERROR_CHECK(g_sniffer->init());
    
    // Start status reporting task
    xTaskCreate(status_task, "status_task", 4096, NULL, 5, NULL);
    
    // Start sniffing on channel 6 (common WiFi channel)
    ESP_LOGI(TAG, "Starting sniffing on channel 6");
    ESP_ERROR_CHECK(g_sniffer->start_sniffing(6));
    
    // Main loop
    while (1) {
        ESP_LOGI(TAG, "Sniffer active on channel %d, Packets: %lu, BT Connected: %s", 
                g_sniffer->get_current_channel(), packet_count,
                g_bluetooth->is_connected() ? "Yes" : "No");
        
        vTaskDelay(pdMS_TO_TICKS(5000)); // Log every 5 seconds
    }
} 