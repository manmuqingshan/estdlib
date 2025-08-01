## chrono

## timer

## unity

(moved out to test/esp-idf/unity)

## Results

These tests are all for variants of Espressif ESP32

|   Date  | Project  | Board                | Chip           | esp-idf  | Result | Notes |
| ------- | -------- | -------------------- | -------------- | -------  | ------ | ----- |
| 05DEC22 | chrono   | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v5.0     | Pass   |
| 08JUN23 | chrono   | ESP32 Lolin Generic  | ESP32          | v5.0.2   | Pass   |
| 31JUL23 | chrono   | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v5.1     | Pass   |
| 02MAR24 | chrono   | Seeed Xiao           | ESP32S3        | v5.1.3   | Pass   |
| 22FEB25 | chrono   | QEMU                 | ESP32          | v5.3.2   | Pass   |
| 22JUL25 | chrono   | QEMU                 | ESP32          | v5.5     | Pass   |
| 07DEC22 | ios      | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v4.4.3   | Pass   |
| 02JAN23 | ios      | ESP32-C3-DevKitM-1   | ESP32C3        | v5.0     | Pass   | 
| 20JUL23 | ios      | ESP32 Lolin Generic  | ESP32          | v5.0.3   | Pass   |
| 19DEC23 | ios      | WaveShare C6-DevKit  | ESP32C6        | v5.1.2   | Pass   |
| 02MAR24 | ios      | Seeed Xiao           | ESP32S3        | v5.1.3   | Pass   |
| 09ARP25 | ios      | QEMU                 | ESP32          | v5.4.1   | Pass   |
| 22FEB25 | ios      | QEMU                 | ESP32S3        | v5.3.2   | Pass   |
| 07DEC22 | timer    | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v4.4.3   | Pass   |
| 15JUN23 | timer    | ESP32-C3-DevKitM-1   | ESP32C3        | v5.0.2   | Pass   |
| 21JUL25 | unity    | QEMU                 | ESP32          | v5.4.2   | Pass   |
| 27JUL25 | unity    | QEMU                 | ESP32          | v5.5     | Pass   |
| 21JUL25 | unity    | QEMU                 | ESP32S3        | v5.4.2   | Pass   |
| 07DEC22 | unity    | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v4.4.3   | Pass   |
| 03JAN23 | unity    | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v5.0     | Pass   |
| 15JUN23 | unity    | ESP32-C3-DevKitM-1   | ESP32C3        | v5.0.2   | Pass   |
| 29SEP23 | unity    | ESP32-C3-DevKitM-1   | ESP32C3        | v5.1.1   | Pass   |
| 19DEC23 | unity    | WaveShare C6-DevKit  | ESP32C6        | v5.1.2   | Pass   |
| 18NOV23 | unity    | ESP32C3 Xiao         | ESP32C3        | v5.1.2   | Pass   |
| 03JAN24 | unity    | ESP32C6 Xiao         | ESP32C6        | v5.3.2   | Pass   | Manual indication of USJ required.  test_freertos_clock fails
| 08APR25 | unity    | ESP32C6 Xiao         | ESP32C6        | v5.4.1   | Pass   | Manual indication of USJ required
| 20JUL23 | unity    | ESP32 Lolin Generic  | ESP32          | v5.0.3   | Pass   |
| 05MAR24 | unity    | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v5.1.2   | Pass   |
| 29SEP23 | unity    | Lilygo QT Pro        | ESP32S3        | v5.1.1   | Pass*  | Intermittent failures[^1]
| 16MAR24 | unity    | Seeed Xiao           | ESP32S3        | v5.1.3   | Pass   |
| 19FEB24 | unity    | ESP-WROVER-KIT v4.1  | ESP32-WROVER-E | v5.2.0   | Pass   |
| 09DEC24 | unity    | UM FeatherS3         | ESP32S3        | v5.3.2   | Pass   |

[^1]: Fails on `test_std_system_clock`.  Likely 18NOV23 commit repairs this

