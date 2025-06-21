# Build Instructions for ESP32 Network Sniffer

This document provides detailed instructions for building and flashing the ESP32 Network Sniffer project.

## Prerequisites

### 1. Install ESP-IDF

#### Windows
```bash
# Download ESP-IDF installer
# https://dl.espressif.com/dl/esp-idf-tools-setup-online-v5.1.1.exe

# Run the installer and follow the prompts
# The installer will set up ESP-IDF v5.1.1 and required tools
```

#### Linux/macOS
```bash
# Clone ESP-IDF
git clone --recursive https://github.com/espressif/esp-idf.git
cd esp-idf

# Install ESP-IDF
./install.sh esp32

# Set up environment (add to ~/.bashrc or ~/.zshrc)
. $HOME/esp/esp-idf/export.sh
```

### 2. Install Required Tools

- **Python 3.7+**: Required for ESP-IDF build system
- **Git**: For version control
- **CMake 3.16+**: Build system
- **Ninja**: Build tool (optional, but recommended)

## Project Setup

### 1. Clone the Repository
```bash
git clone <repository-url>
cd esp32-idf-network-sniffer
```

### 2. Set Up ESP-IDF Environment

#### Windows (Command Prompt)
```cmd
%userprofile%\esp\esp-idf\export.bat
```

#### Windows (PowerShell)
```powershell
%userprofile%\esp\esp-idf\export.ps1
```

#### Linux/macOS
```bash
. $HOME/esp/esp-idf/export.sh
```

### 3. Configure the Project
```bash
# Open configuration menu
idf.py menuconfig
```

#### Important Configuration Options

Navigate to these sections and configure:

**Component config → ESP32-specific → WiFi**
- Set WiFi buffer sizes as needed
- Enable WiFi debugging if required

**Component config → Log output**
- Set default log level to Info or Debug
- Configure log output destination

**Component config → FreeRTOS**
- Adjust task stack sizes if needed
- Configure tick rate (default: 1000Hz)

**Component config → ESP System Settings**
- Configure CPU frequency
- Set flash size

### 4. Build the Project
```bash
# Clean previous builds (optional)
idf.py clean

# Build the project
idf.py build
```

#### Build Output
Successful build will show:
```
Project build complete. To flash, run "idf.py -p (PORT) flash"
```

## Flashing and Monitoring

### 1. Connect ESP32
- Connect ESP32 to your computer via USB
- Note the COM port (Windows) or device path (Linux/macOS)

### 2. Flash the Firmware
```bash
# Flash to ESP32 (replace COM3 with your port)
idf.py -p COM3 flash

# For Linux/macOS, use device path like /dev/ttyUSB0
idf.py -p /dev/ttyUSB0 flash
```

### 3. Monitor Serial Output
```bash
# Monitor serial output
idf.py -p COM3 monitor

# Or flash and monitor in one command
idf.py -p COM3 flash monitor
```

#### Serial Monitor Commands
- `Ctrl+]` - Exit monitor
- `Ctrl+T Ctrl+H` - Show help
- `Ctrl+T Ctrl+R` - Reset target
- `Ctrl+T Ctrl+D` - Reset target into bootloader

## Troubleshooting

### Common Build Issues

#### 1. ESP-IDF Not Found
```bash
# Error: Could not find ESP-IDF
# Solution: Set up environment
. $HOME/esp/esp-idf/export.sh  # Linux/macOS
# or
%userprofile%\esp\esp-idf\export.bat  # Windows
```

#### 2. Python Version Issues
```bash
# Error: Python version not supported
# Solution: Install Python 3.7+ and ensure it's in PATH
python --version
```

#### 3. CMake Version Issues
```bash
# Error: CMake version too old
# Solution: Update CMake to 3.16+
cmake --version
```

### Common Flash Issues

#### 1. Port Not Found
```bash
# Error: Could not open port
# Solution: Check device manager (Windows) or ls /dev/tty* (Linux)
# Ensure ESP32 is connected and drivers are installed
```

#### 2. Permission Denied (Linux)
```bash
# Error: Permission denied on /dev/ttyUSB0
# Solution: Add user to dialout group
sudo usermod -a -G dialout $USER
# Log out and back in, or run:
newgrp dialout
```

#### 3. ESP32 Not in Download Mode
```bash
# Error: Failed to connect to ESP32
# Solution: Hold BOOT button while starting flash
# Or press EN (reset) button
```

### Debugging

#### 1. Enable Verbose Logging
```bash
# In menuconfig: Component config → Log output → Default log verbosity → Verbose
idf.py menuconfig
```

#### 2. Check Build Logs
```bash
# View detailed build output
idf.py build -v
```

#### 3. Monitor with Timestamps
```bash
# Monitor with timestamps
idf.py monitor --timestamp
```

## Advanced Configuration

### 1. Custom Partition Table
Create `partitions.csv`:
```csv
# Name,   Type, SubType, Offset,  Size, Flags
nvs,      data, nvs,     0x9000,  0x6000,
phy_init, data, phy,     0xf000,  0x1000,
factory,  app,  factory, 0x10000, 1M,
```

### 2. Custom sdkconfig
```bash
# Copy default config
cp sdkconfig.defaults sdkconfig

# Edit manually or use menuconfig
idf.py menuconfig
```

### 3. Build Variants
```bash
# Build with specific configuration
idf.py build -D CONFIG_ESP32_DEFAULT_CPU_FREQ_240=1

# Build with custom sdkconfig
idf.py build --config-file custom_sdkconfig
```

## Performance Optimization

### 1. Build Speed
```bash
# Use parallel builds
idf.py build -j 4

# Use ccache (if available)
export IDF_CCACHE_ENABLE=1
```

### 2. Flash Speed
```bash
# Use faster flash speed
idf.py flash --flash-mode dio --flash-freq 40m
```

### 3. Monitor Performance
```bash
# Use higher baud rate
idf.py monitor --baud 115200
```

## Continuous Integration

### GitHub Actions Example
```yaml
name: Build ESP32 Project
on: [push, pull_request]
jobs:
  build:
    runs-on: ubuntu-latest
    steps:
    - uses: actions/checkout@v3
    - uses: espressif/esp-idf-ci-action@v1
      with:
        esp-idf-version: v5.1.1
        target: esp32
    - run: idf.py build
```

## Next Steps

After successful build and flash:

1. **Test Basic Functionality**: Verify serial output shows initialization
2. **Configure WiFi**: Update WiFi credentials if needed
3. **Monitor Packets**: Check for packet capture logs
4. **Customize**: Modify packet processing logic
5. **Deploy**: Use in your target environment

For more information, see the main README.md file. 