#include "network_sniffer.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"

const char* NetworkSniffer::TAG = "NETWORK_SNIFFER";

NetworkSniffer::NetworkSniffer() 
    : current_channel(1), sniffing_active(false), packet_callback(nullptr) {
}

NetworkSniffer::~NetworkSniffer() {
    if (sniffing_active) {
        stop_sniffing();
    }
}

esp_err_t NetworkSniffer::init() {
    ESP_LOGI(TAG, "Initializing network sniffer");
    
    // Register WiFi event handler
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        ESP_EVENT_ANY_ID,
        &NetworkSniffer::wifi_event_handler,
        this,
        &wifi_event_handler_instance
    ));
    
    ESP_LOGI(TAG, "Network sniffer initialized successfully");
    return ESP_OK;
}

esp_err_t NetworkSniffer::start_sniffing(uint8_t channel) {
    if (sniffing_active) {
        ESP_LOGW(TAG, "Sniffing already active");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Starting sniffing on channel %d", channel);
    
    // Set WiFi mode to promiscuous
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_NULL));
    
    // Set channel
    ESP_ERROR_CHECK(esp_wifi_set_channel(channel, WIFI_SECOND_CHAN_NONE));
    
    // Start WiFi
    ESP_ERROR_CHECK(esp_wifi_start());
    
    // Set promiscuous mode
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(true));
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous_rx_cb(&NetworkSniffer::packet_handler));
    
    current_channel = channel;
    sniffing_active = true;
    
    ESP_LOGI(TAG, "Sniffing started on channel %d", channel);
    return ESP_OK;
}

esp_err_t NetworkSniffer::stop_sniffing() {
    if (!sniffing_active) {
        ESP_LOGW(TAG, "Sniffing not active");
        return ESP_ERR_INVALID_STATE;
    }
    
    ESP_LOGI(TAG, "Stopping sniffing");
    
    // Disable promiscuous mode
    ESP_ERROR_CHECK(esp_wifi_set_promiscuous(false));
    
    // Stop WiFi
    ESP_ERROR_CHECK(esp_wifi_stop());
    
    sniffing_active = false;
    
    ESP_LOGI(TAG, "Sniffing stopped");
    return ESP_OK;
}

void NetworkSniffer::set_packet_callback(void (*callback)(const uint8_t* data, size_t len)) {
    packet_callback = callback;
}

uint8_t NetworkSniffer::get_current_channel() const {
    return current_channel;
}

bool NetworkSniffer::is_sniffing() const {
    return sniffing_active;
}

void NetworkSniffer::wifi_event_handler(void* arg, esp_event_base_t event_base,
                                       int32_t event_id, void* event_data) {
    NetworkSniffer* sniffer = static_cast<NetworkSniffer*>(arg);
    
    switch (event_id) {
        case WIFI_EVENT_WIFI_READY:
            ESP_LOGI(TAG, "WiFi ready");
            break;
        case WIFI_EVENT_SCAN_DONE:
            ESP_LOGI(TAG, "Scan done");
            break;
        default:
            ESP_LOGD(TAG, "WiFi event: %ld", event_id);
            break;
    }
}

void NetworkSniffer::packet_handler(void* buf, wifi_promiscuous_pkt_type_t type) {
    if (type != WIFI_PKT_MGMT && type != WIFI_PKT_DATA) {
        return;
    }
    
    wifi_promiscuous_pkt_t* pkt = (wifi_promiscuous_pkt_t*)buf;
    
    // Log packet information
    ESP_LOGI(TAG, "Packet received - Type: %d, Length: %d, Channel: %d, RSSI: %d",
             type, pkt->rx_ctrl.sig_len, pkt->rx_ctrl.channel, pkt->rx_ctrl.rssi);
    
    // Print first few bytes of the packet for debugging
    if (pkt->rx_ctrl.sig_len > 0) {
        ESP_LOG_BUFFER_HEX(TAG, pkt->payload, 
                          pkt->rx_ctrl.sig_len > 32 ? 32 : pkt->rx_ctrl.sig_len);
    }
    
    // TODO: Add more sophisticated packet analysis here
    // - Parse 802.11 frame headers
    // - Extract MAC addresses
    // - Analyze packet types (beacon, probe, data, etc.)
    // - Store statistics
} 