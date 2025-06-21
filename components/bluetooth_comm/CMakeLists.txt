idf_component_register(
    SRCS "bluetooth_comm.cpp"
    INCLUDE_DIRS "include"
    REQUIRES "driver" "esp_bt" "esp_gap_ble_api" "esp_gatts_api" "esp_system" "nvs_flash"
) 