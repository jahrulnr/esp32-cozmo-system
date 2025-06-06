# Cozmo-System DTO Contract

## Pendahuluan

Dokumen ini berfungsi sebagai kontrak resmi untuk format Data Transfer Objects (DTO) antara server Go dan mikrokontroler ESP32-CAM dalam sistem Cozmo-System. Tujuan dari kontrak ini adalah memastikan konsistensi komunikasi dua arah antara komponen-komponen sistem.

## Versi

**Versi Kontrak:** 1.0.0  
**Tanggal Terakhir Diperbarui:** 5 Juni 2025  
**Status:** Draft

## Prinsip Desain

1. **Konsistensi Format** - Semua pesan menggunakan format DTO yang seragam
2. **Versioning** - Semua DTO menyertakan versi untuk kompatibilitas
3. **Validasi** - Format pesan harus dapat divalidasi di kedua sisi
4. **Minimalisasi** - Payload dijaga tetap minimal untuk efisiensi

## Format Dasar

Semua pesan yang dipertukarkan melalui WebSocket mengikuti format dasar berikut:

```json
{
  "version": "1.0",
  "type": "command_type",
  "data": {
    // Payload spesifik untuk tipe pesan
  }
}
```

Di mana:
- `version`: String versi format DTO (wajib)
- `type`: String yang mengidentifikasi jenis pesan (wajib)
- `data`: Objek yang berisi payload utama (wajib)

## Kategori Pesan

Pesan-pesan dalam sistem dibagi menjadi kategori-kategori berikut:

1. **Autentikasi** (`auth_*`) - Login, logout, manajemen sesi
2. **Kontrol Robot** (`robot_*`) - Perintah pergerakan dan aktuator
3. **Sensor** (`sensor_*`) - Data dari sensor robot
4. **Kamera** (`camera_*`) - Streaming video dan pengaturan kamera
5. **File** (`file_*`) - Manajemen file sistem
6. **Sistem** (`system_*`) - Status dan kontrol sistem
7. **WiFi** (`wifi_*`) - Konfigurasi dan status jaringan
8. **Chat** (`chat_*`) - Komunikasi berbasis teks

## Alur Komunikasi

Aliran komunikasi antara komponen sistem adalah sebagai berikut:

```
Browser Client <-> Go Server <-> ESP32-CAM Microcontroller
```

1. **Browser ke Server**: Permintaan pengguna dan UI updates
2. **Server ke ESP32**: Perintah kontrol dan permintaan data
3. **ESP32 ke Server**: Data sensor, status, dan frame kamera
4. **Server ke Browser**: Data terproses dan respons

## Fitur yang Didukung

Berikut ini adalah fitur yang didukung oleh kontrak DTO ini:

- Autentikasi dan manajemen sesi
- Streaming video kamera
- Kontrol motor dan servo
- Pengaturan kamera
- Pembacaan sensor
- Manajemen file
- Konfigurasi WiFi
- Pengiriman perintah teks (chat)
- Notifikasi sistem

## Skema & Contoh

Skema JSON lengkap dan contoh untuk setiap tipe DTO dapat ditemukan di:

- Skema: [./schemas/](./schemas/)
- Contoh: [./examples/](./examples/)

## Implementasi

### Sisi Server (Go)

Pada sisi server, DTO diimplementasikan sebagai struct Go dengan tag JSON:

```go
type BaseDTO struct {
    Version string      `json:"version"`
    Type    string      `json:"type"`
    Data    interface{} `json:"data"`
}
```

### Sisi Mikrokontroler (C++/Arduino)

Pada sisi mikrokontroler, digunakan ArduinoJson untuk parsing dan serialisasi DTO:

```cpp
// Parsing
Utils::SpiJsonDocument doc;
deserializeJson(doc, jsonString);

String version = doc["version"];
String type = doc["type"];
JsonObject data = doc["data"];

// Serialisasi
Utils::SpiJsonDocument doc;
doc["version"] = "1.0";
doc["type"] = "sensor_data";
JsonObject data = doc.createNestedObject("data");
data["temperature"] = 25.5;

String jsonString;
serializeJson(doc, jsonString);
```

## Kompatibilitas Versi

Untuk menjaga kompatibilitas mundur:

1. Field baru harus ditambahkan sebagai opsional
2. Field yang sudah ada tidak boleh diubah maknanya
3. Versi DTO harus diincrement ketika terjadi perubahan yang melanggar kompatibilitas

## Kesimpulan

Kontrak DTO ini berfungsi sebagai rujukan resmi untuk komunikasi antara server dan mikrokontroler. Perubahan pada format pesan harus didokumentasikan di sini dan dikomunikasikan ke semua tim pengembangan.
