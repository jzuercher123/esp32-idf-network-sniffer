#include "bluetooth_comm.h"
#include "esp_bt.h"
#include "esp_gap_ble_api.h"
#include "esp_gatts_api.h"
#include "esp_bt_defs.h"
#include "esp_gatt_common_api.h"
#include "esp_log.h"
#include "esp_system.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/queue.h"
#include <string.h>

const char* BluetoothComm::TAG = "BLUETOOTH_COMM";

// Global instance for static callbacks
static BluetoothComm* g_bt_comm = nullptr;

// Service and characteristic UUIDs
static const uint16_t SNIFFER_SERVICE_UUID = 0x1800;
static const uint16_t SNIFFER_CHAR_UUID = 0x2A00;

// Service and characteristic handles
static uint16_t service_handle;
static uint16_t char_handle;
static esp_gatt_if_t gatts_if = ESP_GATT_IF_NONE;

// Advertising parameters
static esp_ble_adv_params_t adv_params = {
    .adv_int_min = 0x20,
    .adv_int_max = 0x40,
    .adv_type = ADV_TYPE_IND,
    .own_addr_type = BLE_ADDR_TYPE_PUBLIC,
    .channel_map = ADV_CHNL_ALL,
    .adv_filter_policy = ADV_FILTER_ALLOW_SCAN_ANY_CON_ANY,
};

BluetoothComm::BluetoothComm() 
    : device_name("ESP32_Sniffer"), connected(false), conn_id(0) {
    memset(&remote_addr, 0, sizeof(remote_addr));
    data_queue = xQueueCreate(10, sizeof(uint8_t*));
    g_bt_comm = this;
}

BluetoothComm::~BluetoothComm() {
    if (connected) {
        stop_advertising();
    }
    if (data_queue) {
        vQueueDelete(data_queue);
    }
    g_bt_comm = nullptr;
}

esp_err_t BluetoothComm::init() {
    ESP_LOGI(TAG, "Initializing Bluetooth communication");
    
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);
    
    // Initialize Bluetooth controller
    ESP_ERROR_CHECK(esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT));
    
    esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
    ret = esp_bt_controller_init(&bt_cfg);
    if (ret) {
        ESP_LOGE(TAG, "Initialize controller failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = esp_bt_controller_enable(ESP_BT_MODE_BLE);
    if (ret) {
        ESP_LOGE(TAG, "Enable controller failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = esp_bluedroid_init();
    if (ret) {
        ESP_LOGE(TAG, "Init bluedroid failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    ret = esp_bluedroid_enable();
    if (ret) {
        ESP_LOGE(TAG, "Enable bluedroid failed: %s", esp_err_to_name(ret));
        return ret;
    }
    
    // Register GAP callback
    ESP_ERROR_CHECK(esp_ble_gap_register_callback(BluetoothComm::gap_event_handler));
    
    // Register GATTS callback
    ESP_ERROR_CHECK(esp_ble_gatts_register_callback(BluetoothComm::gatts_event_handler));
    
    // Register GATTS application
    ESP_ERROR_CHECK(esp_ble_gatts_app_register(0));
    
    ESP_LOGI(TAG, "Bluetooth communication initialized successfully");
    return ESP_OK;
}

esp_err_t BluetoothComm::start_advertising() {
    ESP_LOGI(TAG, "Starting BLE advertising");
    
    // Set device name
    esp_ble_gap_set_device_name(device_name.c_str());
    
    // Configure advertising data
    esp_ble_adv_data_t adv_data = {
        .set_scan_rsp = false,
        .include_name = true,
        .include_txpower = true,
        .min_interval = 0x20,
        .max_interval = 0x40,
        .appearance = 0x00,
        .manufacturer_len = 0,
        .p_manufacturer_data = NULL,
        .service_data_len = 0,
        .p_service_data = NULL,
        .service_uuid_len = sizeof(SNIFFER_SERVICE_UUID),
        .p_service_uuid = (uint8_t*)&SNIFFER_SERVICE_UUID,
        .flag = (ESP_BLE_ADV_FLAG_GEN_DISC | ESP_BLE_ADV_FLAG_BREDR_NOT_SPT),
    };
    
    ESP_ERROR_CHECK(esp_ble_gap_config_adv_data(&adv_data));
    
    // Start advertising
    esp_ble_gap_start_advertising(&adv_params);
    
    ESP_LOGI(TAG, "BLE advertising started");
    return ESP_OK;
}

esp_err_t BluetoothComm::stop_advertising() {
    ESP_LOGI(TAG, "Stopping BLE advertising");
    esp_ble_gap_stop_advertising();
    return ESP_OK;
}

esp_err_t BluetoothComm::send_data(const uint8_t* data, size_t len) {
    if (!connected) {
        ESP_LOGW(TAG, "Not connected, cannot send data");
        return ESP_ERR_INVALID_STATE;
    }
    
    // Split data into chunks if needed (BLE MTU limit)
    size_t offset = 0;
    while (offset < len) {
        size_t chunk_size = (len - offset > MAX_PACKET_SIZE) ? MAX_PACKET_SIZE : (len - offset);
        
        esp_err_t ret = esp_ble_gatts_send_indicate(gatts_if, conn_id, char_handle, 
                                                   chunk_size, (uint8_t*)&data[offset], false);
        if (ret != ESP_OK) {
            ESP_LOGE(TAG, "Failed to send data chunk: %s", esp_err_to_name(ret));
            return ret;
        }
        
        offset += chunk_size;
        vTaskDelay(pdMS_TO_TICKS(10)); // Small delay between chunks
    }
    
    ESP_LOGD(TAG, "Sent %d bytes of data", len);
    return ESP_OK;
}

esp_err_t BluetoothComm::send_packet_info(uint8_t channel, int8_t rssi, uint16_t length, uint8_t packet_type) {
    if (!connected) {
        return ESP_ERR_INVALID_STATE;
    }
    
    // Create packet info structure
    struct {
        uint8_t channel;
        int8_t rssi;
        uint16_t length;
        uint8_t packet_type;
        uint32_t timestamp;
    } packet_info;
    
    packet_info.channel = channel;
    packet_info.rssi = rssi;
    packet_info.length = length;
    packet_info.packet_type = packet_type;
    packet_info.timestamp = esp_timer_get_time() / 1000; // Convert to milliseconds
    
    return send_data((uint8_t*)&packet_info, sizeof(packet_info));
}

bool BluetoothComm::is_connected() const {
    return connected;
}

void BluetoothComm::set_device_name(const std::string& name) {
    device_name = name;
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
static const uint16_t service_uuid = SNIFFER_SERVICE_UUID;
static const uint16_t char_uuid = SNIFFER_CHAR_UUID; 