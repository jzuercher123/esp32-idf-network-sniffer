idf_component_register(
    SRCS "bluetooth_comm.cpp"
    INCLUDE_DIRS "include"
    REQUIRES "driver" "bt" "esp_system" "nvs_flash"
) 