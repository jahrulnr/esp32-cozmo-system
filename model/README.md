# ESP-SR Model Management Guide

This guide covers flashing ESP-SR models for wake word detection and speech recognition.

> **Source Reference**: This documentation incorporates insights from [Xu Jiwei's comprehensive ESP32-S3 + ESP-SR + ESP-TTS tutorial](https://xujiwei.com/blog/2025/04/esp32-arduino-esp-sr-tts/) which provides excellent troubleshooting guidance for Arduino framework integration with ESP-SR.

## 🚀 Quick Start

### Build and Flash
```bash
# Flash to your ESP32-S3-DevKitC-1-N16R8 (hiesp.csv partition table)
esptool.py --baud 2000000 --before default_reset --after hard_reset  write_flash 0x47D000 model/srmodels.bin
```

# References:
- https://xujiwei.com/blog/2025/04/esp32-arduino-esp-sr-tts/
- https://github.com/espressif/esp-sr