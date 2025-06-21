idf_component_register(
    SRCS "network_sniffer.cpp"
    INCLUDE_DIRS "include"
    REQUIRES "driver" "esp_wifi" "esp_event" "esp_netif" "esp_system" "nvs_flash"
) 