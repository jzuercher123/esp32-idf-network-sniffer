#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "esp_netif.h"
#include "network_sniffer.h"
#include "bluetooth_comm.h"

static const char *TAG = "ESP32_NETWORK_SNIFFER";

// Global instances
NetworkSniffer* g_sniffer = nullptr;
BluetoothComm* g_bluetooth = nullptr;

// Packet statistics
static struct {
    uint32_t total_packets;
    uint32_t management_packets;
    uint32_t data_packets;
    uint32_t bytes_received;
} packet_stats = {0};

// Custom packet processing callback
void packet_processor(const uint8_t* data, size_t len) {
    ESP_LOGI(TAG, "Processing packet of length %d bytes", len);
    
    // Update statistics
    packet_stats.total_packets++;
    packet_stats.bytes_received += len;
    
    // TODO: Add your custom packet processing logic here
    // - Parse specific packet types
    // - Extract useful information
    // - Store data for analysis
    // - Send data to external systems
}

// Enhanced packet handler for Bluetooth transmission
void enhanced_packet_handler(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT && type != WIFI_PKT_DATA) {
        return;
    }
    
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    
    // Update statistics
    if (type == WIFI_PKT_MGMT) {
        packet_stats.management_packets++;
    } else if (type == WIFI_PKT_DATA) {
        packet_stats.data_packets++;
    }
    
    // Log packet information
    ESP_LOGI(TAG, "Packet received - Type: %d, Length: %d, Channel: %d, RSSI: %d",
             type, pkt->rx_ctrl.sig_len, pkt->rx_ctrl.channel, pkt->rx_ctrl.rssi);
    
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
        }
    }
    
    // Print first few bytes of the packet for debugging
    if (pkt->rx_ctrl.sig_len > 0) {
        ESP_LOG_BUFFER_HEX(TAG, pkt->payload, 
                          pkt->rx_ctrl.sig_len > 32 ? 32 : pkt->rx_ctrl.sig_len);
    }
}

// Task to send statistics periodically
void stats_task(void* parameter) {
    while (1) {
        if (g_bluetooth && g_bluetooth->is_connected()) {
            // Create statistics message
            char stats_msg[128];
            snprintf(stats_msg, sizeof(stats_msg), 
                    "STATS: Total=%lu, Mgmt=%lu, Data=%lu, Bytes=%lu",
                    packet_stats.total_packets,
                    packet_stats.management_packets,
                    packet_stats.data_packets,
                    packet_stats.bytes_received);
            
            // Send via Bluetooth
            g_bluetooth->send_data((uint8_t*)stats_msg, strlen(stats_msg));
        }
        
        vTaskDelay(pdMS_TO_TICKS(30000)); // Send stats every 30 seconds
    }
}

extern "C" void app_main(void)
{
    ESP_LOGI(TAG, "ESP32 Network Sniffer with Bluetooth Starting...");
    
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
    g_bluetooth->set_device_name("ESP32_Sniffer");
    ESP_ERROR_CHECK(g_bluetooth->start_advertising());
    ESP_LOGI(TAG, "Bluetooth advertising started");

    // Create and initialize network sniffer
    g_sniffer = new NetworkSniffer();
    ESP_ERROR_CHECK(g_sniffer->init());
    
    // Set packet processing callback
    g_sniffer->set_packet_callback(packet_processor);
    
    // Start statistics task
    xTaskCreate(stats_task, "stats_task", 4096, NULL, 5, NULL);
    
    ESP_LOGI(TAG, "Starting network sniffing on channel 1");
    ESP_ERROR_CHECK(g_sniffer->start_sniffing(1));
    
    // Main application loop
    while (1) {
        ESP_LOGI(TAG, "Network sniffer running on channel %d", g_sniffer->get_current_channel());
        ESP_LOGI(TAG, "Bluetooth connected: %s", g_bluetooth->is_connected() ? "Yes" : "No");
        ESP_LOGI(TAG, "Packets: Total=%lu, Mgmt=%lu, Data=%lu, Bytes=%lu",
                packet_stats.total_packets,
                packet_stats.management_packets,
                packet_stats.data_packets,
                packet_stats.bytes_received);
        
        // Example: Channel hopping every 30 seconds
        static uint8_t current_channel = 1;
        vTaskDelay(pdMS_TO_TICKS(30000)); // Wait 30 seconds
        
        // Switch to next channel (1-13 for 2.4GHz)
        current_channel = (current_channel % 13) + 1;
        ESP_LOGI(TAG, "Switching to channel %d", current_channel);
        g_sniffer->stop_sniffing();
        g_sniffer->start_sniffing(current_channel);
    }
} 