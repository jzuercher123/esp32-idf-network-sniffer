# ESP32 Network Sniffer

A comprehensive ESP32 project for WiFi network packet sniffing and analysis using ESP-IDF framework, with Bluetooth connectivity for Android app integration.

## Features

- **WiFi Promiscuous Mode**: Captures all WiFi packets in the air
- **Channel Hopping**: Automatically switches between WiFi channels (1-13)
- **Packet Analysis**: Basic packet parsing and logging
- **Bluetooth Communication**: BLE GATT server for Android app connectivity
- **Real-time Data Transmission**: Sends packet data and statistics to Android apps
- **Modular Design**: Separate components for network sniffing and Bluetooth communication
- **Configurable**: Easy to customize packet processing and analysis

## Project Structure

```
esp32-idf-network-sniffer/
├── CMakeLists.txt              # Main project CMakeLists.txt
├── sdkconfig.defaults          # Default SDK configuration
├── main/                       # Main application component
│   ├── CMakeLists.txt         # Main component CMakeLists.txt
│   └── main.cpp               # Main application code
├── components/                 # Custom components
│   ├── network_sniffer/       # Network sniffer component
│   │   ├── CMakeLists.txt     # Component CMakeLists.txt
│   │   ├── include/           # Header files
│   │   │   ├── network_sniffer.h
│   │   │   └── README.md
│   │   └── network_sniffer.cpp # Component implementation
│   └── bluetooth_comm/        # Bluetooth communication component
│       ├── CMakeLists.txt     # Component CMakeLists.txt
│       ├── include/           # Header files
│       │   ├── bluetooth_comm.h
│       │   └── README.md
│       └── bluetooth_comm.cpp # Component implementation
├── examples/                   # Example applications
│   ├── basic_sniffer/         # Simple single-channel sniffer
│   ├── channel_hopper/        # Channel hopping example
│   └── bluetooth_sniffer/     # Bluetooth-enabled sniffer
└── README.md                  # This file
```

## Prerequisites

- ESP-IDF v4.4 or later
- ESP32 development board
- USB cable for programming
- WiFi network for testing
- Android device with BLE support (for Bluetooth functionality)

## Setup Instructions

### 1. Install ESP-IDF

Follow the official ESP-IDF installation guide:
https://docs.espressif.com/projects/esp-idf/en/latest/esp32/get-started/

### 2. Clone and Build

```bash
# Clone the repository
git clone <repository-url>
cd esp32-idf-network-sniffer

# Set up ESP-IDF environment
. $HOME/esp/esp-idf/export.sh  # Adjust path as needed

# Configure the project
idf.py menuconfig

# Build the project
idf.py build

# Flash to ESP32
idf.py flash

# Monitor serial output
idf.py monitor
```

### 3. Configuration

The project includes default configurations in `sdkconfig.defaults`. You can customize:

- WiFi buffer sizes
- Bluetooth settings
- Logging levels
- FreeRTOS settings
- Memory configuration
- Network parameters

Use `idf.py menuconfig` to modify settings.

## Usage

### Basic Operation

1. Flash the firmware to your ESP32
2. Connect via serial monitor: `idf.py monitor`
3. The device will start sniffing on channel 1
4. It will automatically hop between channels every 30 seconds
5. Packet information will be logged to the serial console

### Bluetooth Functionality

1. **Device Discovery**: The ESP32 advertises as "ESP32_Sniffer" or "ESP32_Sniffer_BT"
2. **Android Connection**: Use a BLE scanner app or custom Android app to connect
3. **Data Reception**: The Android app receives:
   - Packet information (channel, RSSI, length, type)
   - Raw packet data (first 20 bytes)
   - Statistics updates
   - Status messages

### Android App Integration

#### Required Permissions
```xml
<uses-permission android:name="android.permission.BLUETOOTH" />
<uses-permission android:name="android.permission.BLUETOOTH_ADMIN" />
<uses-permission android:name="android.permission.ACCESS_FINE_LOCATION" />
```

#### Connection Details
- **Service UUID**: `0x1800` (Generic Access Service)
- **Characteristic UUID**: `0x2A00` (Device Name Characteristic)
- **Data Format**: See Bluetooth component documentation

#### Sample Android App Features
- Scan for "ESP32_Sniffer" devices
- Connect to BLE service
- Subscribe to characteristic notifications
- Parse and display packet data
- Show real-time statistics

### Customization

#### Modify Packet Processing

Edit the `packet_processor` function in `main/main.cpp`:

```cpp
void packet_processor(const uint8_t* data, size_t len) {
    // Add your custom packet analysis here
    // Parse specific packet types
    // Extract MAC addresses, SSIDs, etc.
    // Store or transmit data
}
```

#### Change Channel Hopping Behavior

Modify the main loop in `main/main.cpp`:

```cpp
// Change hopping interval (currently 30 seconds)
vTaskDelay(pdMS_TO_TICKS(30000));

// Change channel range (currently 1-13)
current_channel = (current_channel % 13) + 1;
```

#### Customize Bluetooth Data

Modify the Bluetooth transmission in the packet handler:

```cpp
// Send custom data format
if (g_bluetooth && g_bluetooth->is_connected()) {
    // Send your custom data structure
    g_bluetooth->send_data(custom_data, custom_length);
}
```

#### Add New Components

Create new components in the `components/` directory:

```bash
mkdir components/my_component
mkdir components/my_component/include
touch components/my_component/CMakeLists.txt
touch components/my_component/include/my_component.h
touch components/my_component/my_component.cpp
```

## Packet Analysis

The current implementation provides:

### Basic Information
- Packet type (management/data)
- Packet length
- Channel number
- RSSI (signal strength)
- First 32 bytes of packet data (hex dump)

### Bluetooth Transmission
- Real-time packet information
- Formatted statistics
- Status updates
- Raw packet data (limited by BLE MTU)

### Future Enhancements
- 802.11 frame header parsing
- MAC address extraction
- SSID detection from beacon frames
- Packet statistics and counters
- Data export to external systems
- Web interface for monitoring
- Enhanced Android app with visualization

## Examples

### Basic Sniffer
Simple single-channel packet capture without Bluetooth.

### Channel Hopper
Automatic channel switching for comprehensive monitoring.

### Bluetooth Sniffer
Full-featured sniffer with Bluetooth connectivity for Android apps.

## Troubleshooting

### Common Issues

1. **Build Errors**: Ensure ESP-IDF is properly installed and sourced
2. **Flash Errors**: Check USB connection and board selection
3. **No Packets**: Verify WiFi networks are present on the channel
4. **Memory Issues**: Adjust buffer sizes in `sdkconfig.defaults`
5. **Bluetooth Connection Failures**: Check Android app permissions and BLE support

### Debug Output

Enable verbose logging by setting:
```
CONFIG_LOG_DEFAULT_LEVEL_VERBOSE=y
```

### Serial Monitor

Use `idf.py monitor` to view real-time logs and packet information.

## Legal Notice

⚠️ **Important**: This project is for educational and authorized testing purposes only. 

- Only use on networks you own or have explicit permission to test
- Respect privacy and data protection laws
- Do not use for unauthorized network monitoring
- Ensure compliance with local regulations
- Bluetooth communication is unencrypted - not suitable for sensitive data

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Test thoroughly
5. Submit a pull request

## License

This project is licensed under the MIT License - see the LICENSE file for details.

## Support

For issues and questions:
- Check the troubleshooting section
- Review ESP-IDF documentation
- Check Bluetooth component documentation
- Open an issue on GitHub 