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

1. **Autentikasi** - Login, logout, manajemen sesi
2. **Kontrol Robot** - Perintah pergerakan dan aktuator
3. **Sensor** - Data dari sensor robot
4. **Kamera** - Streaming video dan pengaturan kamera
5. **File** - Manajemen file sistem
6. **Sistem** - Status dan kontrol sistem
7. **WiFi** - Konfigurasi dan status jaringan
8. **Chat** - Komunikasi berbasis teks
9. **Log** - Pesan log sistem

> **Catatan**: Dalam implementasi, message type tidak menggunakan prefix kategori (seperti `auth_*`), 
> melainkan menggunakan nama yang langsung menggambarkan aksi (seperti `login`, `motor_command`).
> Lihat dokumentasi MESSAGE_TYPE_MAPPING.md untuk mapping lengkap.

## Alur Komunikasi

Aliran komunikasi antara komponen sistem adalah sebagai berikut:

```
┌─────────────┐      ┌───────────────┐      ┌─────────────┐
│             │      │               │      │             │
│   Browser   │◄────►│  Go Server    │◄────►│  ESP32-CAM  │
│   Client    │      │               │      │             │
└─────────────┘      └───────────────┘      └─────────────┘
```

## Tipe Pesan yang Diimplementasikan

Berikut ini adalah tipe-tipe pesan yang diimplementasi dalam sistem:

### Autentikasi

| Tipe Pesan       | Arah           | Deskripsi                  | Contoh Data |
|------------------|----------------|----------------------------|-------------|
| `login`          | Client → Server | Permintaan login           | `{"username": "admin", "password": "pass123"}` |
| `login_response` | Server → Client | Hasil autentikasi          | `{"success": true, "token": "auth_token_xyz"}` |

### Kontrol Robot

| Tipe Pesan        | Arah           | Deskripsi                  | Contoh Data |
|------------------ |----------------|----------------------------|-------------|
| `motor_command`   | Client → Server | Kontrol motor              | `{"left": 100, "right": 100, "duration": 1000}` |
| `servo_update`    | Client → Server | Posisi servo               | `{"type": "head", "position": 90}` |
| `joystick_update` | Client → Server | Update posisi joystick     | `{"type": "motor", "x": 50, "y": 25}` |

### Sensor

| Tipe Pesan     | Arah           | Deskripsi                  | Contoh Data |
|----------------|----------------|----------------------------|-------------|
| `sensor_data`  | Server → Client | Data dari sensor           | `{"gyro": {"x": 0, "y": 0, "z": 0}, "temperature": 25}` |
| `distance_request` | Client → Server | Request pengukuran jarak | `{}` |

### Kamera

| Tipe Pesan       | Arah           | Deskripsi                  | Contoh Data |
|------------------|----------------|----------------------------|-------------|
| `camera_frame`   | Server → Client | Metadata frame kamera      | `{"width": 640, "height": 480, "format": "jpeg"}` |
| `camera_command` | Client → Server | Kontrol kamera             | `{"action": "start", "resolution": "vga"}` |

### File

| Tipe Pesan       | Arah           | Deskripsi                  | Contoh Data |
|------------------|----------------|----------------------------|-------------|
| `list_files`     | Server → Client | Daftar file                | `[{"name": "config.json", "size": 1024, "type": "file"}]` |
| `read_file`      | Client → Server | Baca file                  | `{"path": "/config/wifi.json"}` |
| `file_operation` | Server → Client | Hasil operasi file         | `{"success": true, "message": "File created"}` |

### Sistem

| Tipe Pesan       | Arah           | Deskripsi                  | Contoh Data |
|------------------|----------------|----------------------------|-------------|
| `system_status`  | Server → Client | Status sistem              | `{"memory": "32KB", "uptime": 3600}` |
| `error`          | Server → Client | Pesan error                | `{"code": 404, "message": "File not found"}` |

### WiFi

| Tipe Pesan        | Arah           | Deskripsi                  | Contoh Data |
|-------------------|----------------|----------------------------|-------------|
| `wifi_list`       | Server → Client | Daftar jaringan WiFi       | `[{"ssid": "Network1", "rssi": -50}]` |
| `connect_wifi`    | Client → Server | Request koneksi WiFi       | `{"ssid": "Network1", "password": "pass123"}` |
| `wifi_connection` | Server → Client | Status koneksi WiFi        | `{"connected": true, "ip": "192.168.1.100"}` |

### Log

| Tipe Pesan          | Arah           | Deskripsi                  | Contoh Data |
|---------------------|----------------|----------------------------|-------------|
| `log_message`       | Server → Client | Pesan log tunggal          | `{"level": "info", "message": "System started"}` |
| `batch_log_messages`| Server → Client | Kumpulan pesan log         | `{"logs": [{"level": "info", "message": "Log 1"}]}` |

> **Catatan:** Untuk informasi lebih detail dan contoh lengkap, lihat folder `/docs/dto_contract/examples/`.

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
