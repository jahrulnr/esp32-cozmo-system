# ESP-SR Model Management Guide

This guide covers flashing ESP-SR models for wake word detection and speech recognition.

> **Source Reference**: This documentation incorporates insights from [Xu Jiwei's comprehensive ESP32-S3 + ESP-SR + ESP-TTS tutorial](https://xujiwei.com/blog/2025/04/esp32-arduino-esp-sr-tts/) which provides excellent troubleshooting guidance for Arduino framework integration with ESP-SR.

## 🚀 Quick Start

### Build and Flash
```bash
# Flash to your ESP32-S3-DevKitC-1-N16R8 (hiesp.csv partition table)

# Flash spiffs
esptool.py --baud 2000000 --before default_reset --after hard_reset  write_flash 0x310000 ../.pio/build/esp32s3dev/spiffs.bin

# Flash model
esptool.py --baud 2000000 --before default_reset --after hard_reset  write_flash 0x8F0000 srmodels.bin
```

# References:
- https://xujiwei.com/blog/2025/04/esp32-arduino-esp-sr-tts/
- https://github.com/espressif/esp-sr