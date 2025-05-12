# Meatloaf File Flasher (MLFF)

**A custom bootloader and update utility for ESP32-based projects**

The **Meatloaf File Flasher (MLFF)** is a lightweight, open-source tool designed to simplify firmware and data partition updates for ESP32 microcontrollers. With MLFF, you can flash your ESP32-based projects directly from an SD card, eliminating the need for a computer, USB connection, or web-based flashing tools. Whether you're deploying updates in the field or prototyping a new project, MLFF makes the process fast, reliable, and user-friendly.

## Features

- **SD Card-Based Updates**: Flash firmware and data partitions by placing `.bin` files on an SD card.
- **No Computer Required**: Perform updates without USB connections or external software.
- **Simple Configuration**: Trigger updates using a single Non-Volatile Storage (NVS) flag.
- **Custom Bootloader**: Ensures reliable and secure firmware loading.
- **Lightweight and Efficient**: Optimized for ESP32's resource-constrained environment, requiring only a 384KB partition for the update app, unlike standard OTA updates that need two full firmware copies in flash.
- **Open-Source**: Freely available for customization and community contributions.

## How It Works

MLFF consists of two main components:

1. **Custom Bootloader**: A specialized bootloader that checks for update conditions during the ESP32 boot process and initiates flashing if required.
2. **Update Utility**: A compact program (using only a 384KB partition) that reads `.bin` files from the SD card and writes them to the appropriate ESP32 partitions (e.g., firmware, filesystem, or custom data partitions). Unlike standard OTA updates, which require two copies of the firmware in flash for redundancy, MLFF's efficient design minimizes flash usage.

To perform an update:

1. Place your firmware or data partition `.bin` files in the `/sd/.bin` folder on an SD card.
2. Insert the SD card into the ESP32's SD card slot.
3. Ensure the `update` flag in the `system` namespace of the ESP32's NVS is set to `1` (default in the provided `nvs.csv`).
4. Reboot the ESP32 to trigger the bootloader and start the flashing process.

The bootloader detects the `update` flag in the `system` namespace, locates the `.bin` files on the SD card, and flashes them to the designated partitions. The update utility initializes the SD card using GPIO pins specified in the `sdcard_pins` NVS key (default: `0x28262729` in hex, corresponding to GPIO pins 40, 38, 39, 41 for MISO, MOSI, SCK, CS). Once complete, the device restarts with the new firmware or data.

## Getting Started

### Prerequisites

- An ESP32-based development board with an SD card slot (e.g., ESP32-WROOM, ESP32-S3).
- An SD card formatted as FAT32 (up to 32GB recommended).
- `.bin` files for your firmware or data partitions (generated using ESP-IDF or Arduino IDE).
- Basic knowledge of ESP32 programming and NVS configuration.

### Installation

1. **Clone the Repository**:

   ```bash
   git clone https://github.com/idolpx/mlff.git
   ```

2. **Set the Target and Build the Bootloader and Utility**:

   - Use the ESP-IDF framework to compile the MLFF bootloader and update utility.

   - Navigate to the `mlff` directory.

   - Set the target for your ESP32 chip (e.g., `esp32` or `esp32s3`):

     ```bash
     idf.py set-target esp32
     ```

     Replace `esp32` with `esp32s3` if using an ESP32-S3 board.

   - Build the project:

     ```bash
     idf.py build
     ```

3. **Flash MLFF to Your ESP32**:

   - Flash the compiled bootloader and utility to your ESP32 using:

     ```bash
     idf.py -p PORT flash
     ```

     Replace `PORT` with your ESP32's serial port (e.g., `/dev/ttyUSB0` on Linux or `COM3` on Windows).

4. **Prepare the SD Card**:

   - Create a folder named `.bin` in the root of the SD card (`/sd/.bin`).
   - Copy your `.bin` files (e.g., `main.v1.0.2025.bin`, `storage.latest.bin`) to this folder.

5. **Configure NVS**:

   - The provided `nvs.csv` file sets default values for the `system` namespace:

     - `update`: Set to `1` to enable updates by default.
     - `sdcard_pins`: Set to `0x28262729` (hex), defining GPIO pins 40, 38, 39, 41 for MISO, MOSI, SCK, CS (in that order) for SD card initialization.

   - To apply these NVS settings, generate an NVS binary from the `nvs.csv` file and flash it to the NVS partition:

     ```bash
     python components/nvs_flash/nvs_partition_gen.py --input nvs.csv --output nvs.bin --size NVS_PARTITION_SIZE
     esptool.py --port PORT write_flash NVS_PARTITION_ADDRESS nvs.bin
     ```

     Replace `NVS_PARTITION_SIZE` with the size of your NVS partition (e.g., `0x6000`), `NVS_PARTITION_ADDRESS` with the partition offset (e.g., `0x9000`), and `PORT` with your serial port.

   - To manually modify the `update` flag (if needed):

     ```bash
     python components/nvs_flash/nvs_partition_tool.py --port PORT write system update 1
     ```

   - To modify the `sdcard_pins` (if using different GPIO pins):

     ```bash
     python components/nvs_flash/nvs_partition_tool.py --port PORT write system sdcard_pins hex2bin NEW_PIN_HEX
     ```

     Replace `NEW_PIN_HEX` with the hex-encoded pin configuration (e.g., `28262729` for pins 40, 38, 39, 41 for MISO, MOSI, SCK, CS).

### Usage

1. Insert the prepared SD card into the ESP32's SD card slot.
2. Power on or reset the ESP32.
3. The bootloader checks the `update` flag in the `system` namespace:
   - If set to `1` (default in `nvs.csv`), it scans `/sd/.bin` for `.bin` files and flashes them to the corresponding partitions.
   - If set to `0` or unset, it boots the existing firmware.
4. The update utility initializes the SD card using the GPIO pins specified in the `sdcard_pins` key.
5. After flashing, the bootloader resets the `update` flag in the `system` namespace to `0` and restarts the ESP32 with the updated firmware.

## File Naming and Partition Mapping

- MLFF requires the part of the `.bin` file name before the first `.` to match the target partition name as defined in your ESP32 project's partition table (see `partitions.csv` in ESP-IDF). MLFF uses the following default partition names:
  - `main`: Main firmware application.
  - `update`: MLFF update app (requires a 384KB partition).
  - `storage`: Flash filesystem (e.g., SPIFFS or LittleFS).
- Anything after the first `.` in the file name is ignored and can be used to specify additional information such as model, revision, date, or other identifiers.
- Examples:
  - `main.v1.0.2025.bin`: Flashes to the `main` partition (main application firmware).
  - `update.v2.1.bin`: Flashes to the `update` partition (MLFF update app).
  - `storage.modelX.20231001.bin`: Flashes to the `storage` partition (flash filesystem).
- Ensure your `.bin` files align with the partition table to avoid flashing errors.
- MLFF validates file integrity before flashing to prevent corruption.

## Example Workflow

1. Build your firmware using ESP-IDF or Arduino IDE, generating `.bin` files like `main.v1.0.bin` and `storage.v2.0.bin`.
2. Copy these files to `/sd/.bin` on the SD card.
3. Insert the SD card into the ESP32.
4. Ensure the NVS is configured with the default `nvs.csv` settings (or manually set the `update` flag in the `system` namespace to `1`):

   ```bash
   python set_nvs.py --port /dev/ttyUSB0 --namespace system --key update --value 1
   ```

5. Reboot the ESP32. MLFF flashes the files to the corresponding partitions and restarts the device with the new firmware.

## Troubleshooting

- **Update Fails to Start**:
  - Verify the `update` flag in the `system` namespace is set to `1` using an NVS reader.
  - Ensure the SD card is formatted as FAT32 and the `.bin` files are in `/sd/.bin`.
- **SD Card Initialization Fails**:
  - Confirm the `sdcard_pins` in the `system` namespace matches your hardware (default: `0x28262729` for GPIO 40, 38, 39, 41 for MISO, MOSI, SCK, CS).
  - Check the SD card slot wiring and GPIO pin compatibility with your ESP32 model.
- **Flashing Errors**:
  - Check that the `.bin` file names (before the first `.`) match the partition names (`main`, `update`, or `storage`) in your partition table.
  - Confirm the SD card is properly seated and readable by the ESP32.
- **Device Doesn’t Boot**:
  - Re-flash the MLFF bootloader and utility using ESP-IDF.
  - Verify the bootloader `.bin` (if updated) is compatible with your ESP32 model.
- For further assistance, check the [Issues](https://github.com/idolpx/mlff/issues) page or join our [Discord community](https://discord.gg/FwJUe8kQpS).

## Contributing

We welcome contributions to MLFF! To get started:

1. Fork the repository and create a new branch for your feature or bug fix.
2. Submit a pull request with a clear description of your changes.
3. Follow the coding style and include tests where applicable.

See [CONTRIBUTING.md](CONTRIBUTING.md) for more details.

## License

MLFF is licensed under the [GNU General Public License v3.0](LICENSE). See the [LICENSE](LICENSE) file for details.

## Acknowledgments

- Built with the [ESP-IDF](https://github.com/espressif/esp-idf) framework.
- Inspired by the ESP32 community's need for a simple, offline flashing solution.
- Thanks to all contributors and testers who helped shape MLFF!

## Contact

For questions, suggestions, or bug reports, open an issue on the [GitHub repository](https://github.com/idolpx/mlff) or join our [Discord community](https://discord.gg/FwJUe8kQpS).