# Cozmo-System DTO Contract Changelog

Dokumen ini mencatat semua perubahan penting dalam format Data Transfer Object (DTO) untuk komunikasi antara server Go dan mikrokontroler ESP32-CAM dalam sistem Cozmo-System.

## v1.0.0 - 5 Juni 2025

### Ditambahkan
- Format dasar DTO dengan field `version`, `type`, dan `data`
- Skema JSON untuk kategori pesan:
  - Authentication (auth_*)
  - Robot Control (robot_*)
  - Camera (camera_*)
  - Sensors (sensor_*)
  - System (system_*)
  - WiFi (wifi_*)
- Dokumentasi untuk implementasi di server Go dan mikrokontroler
- Contoh pesan untuk semua kategori
- Panduan implementasi untuk frontend, server, dan mikrokontroler

### Perubahan
- Menambahkan field `version` ke semua pesan DTO (sebelumnya hanya memiliki `type` dan `data`)
- Standardisasi nama tipe pesan dengan format `category_action` (contoh: `robot_motor`, `camera_start`)
- Struktur data yang lebih konsisten untuk semua jenis pesan

### Perbaikan
- Validasi yang lebih baik dengan skema JSON
- Format yang lebih jelas untuk data biner (seperti frame kamera)
- Menangani transaksi multi-pesan dengan lebih baik (seperti upload file)

## Pra-v1.0.0 - Sebelum Juni 2025

Format DTO sebelumnya menggunakan struktur JSON tanpa versioning:

```json
{
  "type": "command_type",
  "data": {
    // Payload
  }
}
```

Detil lain tentang format lama dapat dilihat di [dokumentasi DTO lama](/docs/DTO_FORMAT.md).
