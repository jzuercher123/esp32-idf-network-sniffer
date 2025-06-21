#pragma once

#include <string>
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"

class NetworkSniffer {
public:
    NetworkSniffer();
    ~NetworkSniffer();
    
    // Initialize the network sniffer
    esp_err_t init();
    
    // Start sniffing on a specific channel
    esp_err_t start_sniffing(uint8_t channel);
    
    // Stop sniffing
    esp_err_t stop_sniffing();
    
    // Set callback for packet processing
    void set_packet_callback(void (*callback)(const uint8_t* data, size_t len));
    
    // Get current channel
    uint8_t get_current_channel() const;
    
    // Check if sniffing is active
    bool is_sniffing() const;

private:
    static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                                  int32_t event_id, void* event_data);
    
    static void packet_handler(void* buf, wifi_promiscuous_pkt_type_t type);
    
    // WiFi event handler instance
    esp_event_handler_instance_t wifi_event_handler_instance;
    
    // Current channel being monitored
    uint8_t current_channel;
    
    // Sniffing state
    bool sniffing_active;
    
    // Packet callback function
    void (*packet_callback)(const uint8_t* data, size_t len);
    
    // Log tag
    static const char* TAG;
}; 