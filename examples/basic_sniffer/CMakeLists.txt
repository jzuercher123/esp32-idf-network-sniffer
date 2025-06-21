idf_component_register(
    SRCS "main.cpp"
    INCLUDE_DIRS "."
    REQUIRES "driver" "esp_wifi" "esp_event" "esp_netif" "esp_system" "nvs_flash" "network_sniffer"
) 