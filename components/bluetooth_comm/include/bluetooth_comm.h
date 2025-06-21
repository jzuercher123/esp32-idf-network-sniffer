#pragma once

#include <string>
#include <vector>
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "freertos/FreeRTOS.h"
#include "freertos/queue.h"

class BluetoothComm {
public:
    BluetoothComm();
    ~BluetoothComm();
    
    // Initialize Bluetooth communication
    esp_err_t init();
    
    // Start advertising
    esp_err_t start_advertising();
    
    // Stop advertising
    esp_err_t stop_advertising();
    
    // Send data to connected Android app
    esp_err_t send_data(const uint8_t* data, size_t len);
    
    // Send packet information
    esp_err_t send_packet_info(uint8_t channel, int8_t rssi, uint16_t length, uint8_t packet_type);
    
    // Check if device is connected
    bool is_connected() const;
    
    // Set device name
    void set_device_name(const std::string& name);
    
    // Get connection status
    esp_gatt_conn_desc_t* get_connection_info();

private:
    // GAP event handler
    static void gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param);
    
    // GATTS event handler
    static void gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
    
    // GATTS profile event handler
    static void gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param);
    
    // Initialize GATT service
    esp_err_t init_gatt_service();
    
    // Update characteristic value
    esp_err_t update_characteristic_value(uint16_t char_handle, const uint8_t* data, size_t len);
    
    // Device name
    std::string device_name;
    
    // Connection state
    bool connected;
    uint16_t conn_id;
    esp_bd_addr_t remote_addr;
    
    // GATT service and characteristic handles
    uint16_t service_handle;
    uint16_t char_handle;
    esp_gatt_if_t gatts_if;
    
    // Data queue for sending
    QueueHandle_t data_queue;
    
    // Log tag
    static const char* TAG;
    
    // Service and characteristic UUIDs
    static const uint16_t SERVICE_UUID;
    static const uint16_t CHAR_UUID;
    
    // Maximum packet size for BLE
    static const size_t MAX_PACKET_SIZE = 20; // BLE MTU limit
};

// Custom service and characteristic UUIDs
#define SNIFFER_SERVICE_UUID    0x1800  // Generic Access Service
#define SNIFFER_CHAR_UUID       0x2A00  // Device Name Characteristic 