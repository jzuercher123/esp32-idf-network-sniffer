# Bluetooth Communication Component

This component provides BLE (Bluetooth Low Energy) communication functionality for sending network sniffer data to Android applications.

## Features

- **BLE GATT Server**: Implements a GATT server for Android app connectivity
- **Automatic Advertising**: Starts BLE advertising for device discovery
- **Data Transmission**: Sends packet information and raw data to connected devices
- **Connection Management**: Handles connection/disconnection events
- **Statistics Reporting**: Periodically sends sniffer statistics

## API Reference

### BluetoothComm Class

#### Constructor
```cpp
BluetoothComm();
```
Creates a new BluetoothComm instance.

#### Destructor
```cpp
~BluetoothComm();
```
Automatically stops advertising and cleans up resources.

#### Methods

##### `esp_err_t init()`
Initializes the Bluetooth communication component.
- **Returns**: `ESP_OK` on success, error code on failure

##### `esp_err_t start_advertising()`
Starts BLE advertising for device discovery.
- **Returns**: `ESP_OK` on success, error code on failure

##### `esp_err_t stop_advertising()`
Stops BLE advertising.
- **Returns**: `ESP_OK` on success, error code on failure

##### `esp_err_t send_data(const uint8_t* data, size_t len)`
Sends raw data to the connected Android app.
- **Parameters**: 
  - `data` - Pointer to data buffer
  - `len` - Length of data in bytes
- **Returns**: `ESP_OK` on success, error code on failure

##### `esp_err_t send_packet_info(uint8_t channel, int8_t rssi, uint16_t length, uint8_t packet_type)`
Sends formatted packet information to the Android app.
- **Parameters**:
  - `channel` - WiFi channel number
  - `rssi` - Signal strength indicator
  - `length` - Packet length in bytes
  - `packet_type` - Type of packet (management/data)
- **Returns**: `ESP_OK` on success, error code on failure

##### `bool is_connected() const`
Checks if an Android device is connected.
- **Returns**: `true` if connected, `false` otherwise

##### `void set_device_name(const std::string& name)`
Sets the BLE device name for advertising.
- **Parameters**: `name` - Device name string

## Usage Example

```cpp
#include "bluetooth_comm.h"

// Create Bluetooth instance
BluetoothComm bluetooth;

// Initialize
esp_err_t ret = bluetooth.init();
if (ret != ESP_OK) {
    ESP_LOGE(TAG, "Failed to initialize Bluetooth");
    return;
}

// Set device name and start advertising
bluetooth.set_device_name("ESP32_Sniffer");
bluetooth.start_advertising();

// Send data when connected
if (bluetooth.is_connected()) {
    // Send packet information
    bluetooth.send_packet_info(6, -45, 64, WIFI_PKT_MGMT);
    
    // Send raw data
    uint8_t data[] = {0x01, 0x02, 0x03, 0x04};
    bluetooth.send_data(data, sizeof(data));
}
```

## Android App Integration

### Service UUID
- **Service UUID**: `0x1800` (Generic Access Service)
- **Characteristic UUID**: `0x2A00` (Device Name Characteristic)

### Data Format

#### Packet Information Structure
```cpp
struct {
    uint8_t channel;      // WiFi channel (1-13)
    int8_t rssi;          // Signal strength (-100 to 0 dBm)
    uint16_t length;      // Packet length in bytes
    uint8_t packet_type;  // WIFI_PKT_MGMT or WIFI_PKT_DATA
    uint32_t timestamp;   // Timestamp in milliseconds
} packet_info;
```

#### Status Messages
- Format: `"STATUS: Packets=X, Channel=Y, Connected=Yes/No"`
- Format: `"STATS: Total=X, Mgmt=Y, Data=Z, Bytes=W"`

### Android App Requirements

1. **BLE Permissions**: Add to AndroidManifest.xml
```xml
<uses-permission android:name="android.permission.BLUETOOTH" />
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
```

2. **Scan for Device**: Look for device named "ESP32_Sniffer" or "ESP32_Sniffer_BT"

3. **Connect to Service**: Connect to service UUID `0x1800`

4. **Subscribe to Notifications**: Enable notifications on characteristic `0x2A00`

## Configuration

### SDK Configuration
Add to `sdkconfig.defaults`:
```
# Bluetooth Configuration
CONFIG_BT_ENABLED=y
CONFIG_BT_BLUEDROID_ENABLED=y
CONFIG_BT_BLE_ENABLED=y
CONFIG_BT_GATTS_ENABLE=y
CONFIG_BT_GATTC_ENABLE=n
```

### Memory Configuration
Bluetooth requires additional memory. Ensure sufficient heap:
```
CONFIG_ESP32_SPIRAM_SUPPORT=y
CONFIG_SPIRAM_USE_MALLOC=y
```

## Troubleshooting

### Common Issues

1. **Build Errors**: Ensure Bluetooth components are enabled in ESP-IDF
2. **Connection Failures**: Check Android app permissions and BLE support
3. **Data Loss**: BLE has MTU limits (20 bytes), large data is automatically chunked
4. **Memory Issues**: Bluetooth requires significant RAM, enable SPIRAM if needed

### Debug Output
Enable Bluetooth debugging:
```
CONFIG_BT_LOG_HCI_TRACE_LEVEL_WARNING=y
CONFIG_BT_LOG_BTM_TRACE_LEVEL_WARNING=y
CONFIG_BT_LOG_GATT_TRACE_LEVEL_WARNING=y
```

## Performance Considerations

- **Data Rate**: BLE has limited bandwidth (~1 Mbps theoretical)
- **Packet Size**: Maximum 20 bytes per BLE packet (MTU limit)
- **Connection Interval**: Adjustable for power vs. latency trade-off
- **Memory Usage**: ~50KB RAM for Bluetooth stack

## Security Notes

- **No Encryption**: This implementation uses unencrypted BLE communication
- **Public Service**: Uses standard GATT service UUIDs
- **No Authentication**: Any BLE device can connect
- **Production Use**: Add encryption and authentication for production deployments 