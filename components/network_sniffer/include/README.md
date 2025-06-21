# Network Sniffer Component

This component provides WiFi packet sniffing functionality for ESP32 using promiscuous mode.

## API Reference

### NetworkSniffer Class

#### Constructor
```cpp
NetworkSniffer();
```
Creates a new NetworkSniffer instance.

#### Destructor
```cpp
~NetworkSniffer();
```
Automatically stops sniffing if active.

#### Methods

##### `esp_err_t init()`
Initializes the network sniffer component.
- **Returns**: `ESP_OK` on success, error code on failure

##### `esp_err_t start_sniffing(uint8_t channel)`
Starts packet sniffing on the specified channel.
- **Parameters**: `channel` - WiFi channel (1-13 for 2.4GHz)
- **Returns**: `ESP_OK` on success, error code on failure

##### `esp_err_t stop_sniffing()`
Stops packet sniffing.
- **Returns**: `ESP_OK` on success, error code on failure

##### `void set_packet_callback(void (*callback)(const uint8_t* data, size_t len))`
Sets a callback function for packet processing.
- **Parameters**: `callback` - Function pointer to packet processing callback

##### `uint8_t get_current_channel() const`
Gets the current channel being monitored.
- **Returns**: Current channel number

##### `bool is_sniffing() const`
Checks if sniffing is currently active.
- **Returns**: `true` if sniffing, `false` otherwise

## Usage Example

```cpp
#include "network_sniffer.h"

// Create sniffer instance
NetworkSniffer sniffer;

// Initialize
esp_err_t ret = sniffer.init();
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize sniffer");
    return;
}

// Set packet callback
sniffer.set_packet_callback([](const uint8_t* data, size_t len) {
    ESP_LOGI(TAG, "Received packet: %d bytes", len);
    // Process packet data here
});

// Start sniffing on channel 6
ret = sniffer.start_sniffing(6);
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to start sniffing");
    return;
}

// Later, stop sniffing
sniffer.stop_sniffing();
```

## Packet Information

The component captures and logs:
- Packet type (management/data)
- Packet length
- Channel number
- RSSI (signal strength)
- First 32 bytes of packet data (hex dump)

## Dependencies

- `driver` - ESP32 driver framework
- `esp_wifi` - WiFi functionality
- `esp_event` - Event handling
- `esp_netif` - Network interface
- `esp_system` - System functions
- `nvs_flash` - Non-volatile storage

## Configuration

The component uses default WiFi configurations. You can customize:
- Buffer sizes
- Channel settings
- Promiscuous mode parameters

See `sdkconfig.defaults` for configuration options. 