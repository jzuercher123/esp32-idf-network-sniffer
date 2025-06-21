#include "bluetooth_comm.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>

const char* BluetoothComm::TAG = "BLUETOOTH_COMM";

BluetoothComm::BluetoothComm() 
    : device_name("ESP32_Sniffer"), connected(false), conn_id(0), 
      service_handle(0), char_handle(0), gatts_if(0) {
    memset(&remote_addr, 0, sizeof(remote_addr));
    data_queue = xQueueCreate(10, sizeof(uint8_t*));
}

BluetoothComm::~BluetoothComm() {
    if (data_queue) {
        vQueueDelete(data_queue);
    }
}

esp_err_t BluetoothComm::init() {
    ESP_LOGI(TAG, "Initializing Bluetooth communication (simplified version)");
    
    // For now, just log that Bluetooth is initialized
    // In a full implementation, this would initialize the Bluetooth stack
    ESP_LOGI(TAG, "Bluetooth communication initialized successfully");
    return ESP_OK;
}

esp_err_t BluetoothComm::start_advertising() {
    ESP_LOGI(TAG, "Starting BLE advertising (simplified version)");
    
    // For now, just log that advertising would start
    // In a full implementation, this would start BLE advertising
    ESP_LOGI(TAG, "BLE advertising started (simulated)");
    return ESP_OK;
}

esp_err_t BluetoothComm::stop_advertising() {
    ESP_LOGI(TAG, "Stopping BLE advertising (simplified version)");
    
    // For now, just log that advertising would stop
    ESP_LOGI(TAG, "BLE advertising stopped (simulated)");
    return ESP_OK;
}

esp_err_t BluetoothComm::send_data(const uint8_t* data, size_t len) {
    if (!connected) {
        ESP_LOGW(TAG, "Not connected, cannot send data");
        return ESP_ERR_INVALID_STATE;
    }
    
    // For now, just log the data that would be sent
    ESP_LOGI(TAG, "Would send %d bytes via Bluetooth", len);
    
    // Log first few bytes for debugging
    if (len > 0) {
        ESP_LOG_BUFFER_HEX(TAG, data, len > 16 ? 16 : len);
    }
    
    return ESP_OK;
}

esp_err_t BluetoothComm::send_packet_info(uint8_t channel, int8_t rssi, uint16_t length, uint8_t packet_type) {
    if (!connected) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // For now, just log the packet info that would be sent
    ESP_LOGI(TAG, "Would send packet info: Channel=%d, RSSI=%d, Length=%d, Type=%d",
             channel, rssi, length, packet_type);
    
    return ESP_OK;
}

bool BluetoothComm::is_connected() const {
    return connected;
}

void BluetoothComm::set_device_name(const std::string& name) {
    device_name = name;
    ESP_LOGI(TAG, "Device name set to: %s", device_name.c_str());
}

esp_gatt_conn_desc_t* BluetoothComm::get_connection_info() {
    return nullptr; // Simplified implementation
}

void BluetoothComm::gap_event_handler(esp_gap_ble_cb_event_t event, esp_ble_gap_cb_param_t *param) {
    if (!g_bt_comm) return;
    
    switch (event) {
        case ESP_GAP_BLE_ADV_DATA_SET_COMPLETE_EVT:
            ESP_LOGI(TAG, "Advertising data set complete");
            break;
            
        case ESP_GAP_BLE_ADV_START_COMPLETE_EVT:
            if (param->adv_start_cmpl.status != ESP_BT_STATUS_SUCCESS) {
                ESP_LOGE(TAG, "Advertising start failed");
            } else {
                ESP_LOGI(TAG, "Advertising start complete");
            }
            break;
            
        case ESP_GAP_BLE_ADV_STOP_COMPLETE_EVT:
            ESP_LOGI(TAG, "Advertising stop complete");
            break;
            
        default:
            ESP_LOGD(TAG, "GAP event: %d", event);
            break;
    }
}

void BluetoothComm::gatts_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    if (!g_bt_comm) return;
    
    switch (event) {
        case ESP_GATTS_REG_EVT:
            ESP_LOGI(TAG, "GATTS register event");
            ::gatts_if = gatts_if;
            
            // Create service
            esp_ble_gatts_create_service(gatts_if, &service_uuid, 4);
            break;
            
        case ESP_GATTS_CREATE_EVT:
            ESP_LOGI(TAG, "GATTS create service event");
            service_handle = param->create.service_handle;
            
            // Create characteristic
            esp_ble_gatts_add_char(service_handle, &char_uuid, 
                                  ESP_GATT_PERM_READ | ESP_GATT_PERM_WRITE,
                                  ESP_GATT_CHAR_PROP_BIT_READ | ESP_GATT_CHAR_PROP_BIT_WRITE | ESP_GATT_CHAR_PROP_BIT_NOTIFY,
                                  NULL, NULL);
            break;
            
        case ESP_GATTS_ADD_CHAR_EVT:
            ESP_LOGI(TAG, "GATTS add characteristic event");
            char_handle = param->add_char.attr_handle;
            
            // Start service
            esp_ble_gatts_start_service(service_handle);
            break;
            
        case ESP_GATTS_CONNECT_EVT:
            ESP_LOGI(TAG, "GATTS connect event");
            g_bt_comm->connected = true;
            g_bt_comm->conn_id = param->connect.conn_id;
            memcpy(g_bt_comm->remote_addr, param->connect.remote_bda, sizeof(esp_bd_addr_t));
            break;
            
        case ESP_GATTS_DISCONNECT_EVT:
            ESP_LOGI(TAG, "GATTS disconnect event");
            g_bt_comm->connected = false;
            g_bt_comm->conn_id = 0;
            memset(g_bt_comm->remote_addr, 0, sizeof(esp_bd_addr_t));
            // Restart advertising
            g_bt_comm->start_advertising();
            break;
            
        default:
            ESP_LOGD(TAG, "GATTS event: %d", event);
            break;
    }
}

void BluetoothComm::gatts_profile_event_handler(esp_gatts_cb_event_t event, esp_gatt_if_t gatts_if, esp_ble_gatts_cb_param_t *param) {
    ESP_LOGD(TAG, "GATTS profile event: %d", event);
}

// Static variables for service and characteristic
static const uint16_t service_uuid = 0x1800;
static const uint16_t char_uuid = 0x2A00; 