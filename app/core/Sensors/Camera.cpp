#include "Camera.h"
#include "Config.h"
#include <esp_log.h>

namespace Sensors {

Camera::Camera() : _resolution(CAMERA_FRAME_SIZE), _initialized(false), TAG("Camera") {
}

Camera::~Camera() {
    if (_initialized) {
        esp_camera_deinit();
    }
}

bool Camera::init() {
    #if CAMERA_ENABLED == false
    return false;
    #else
    camera_config_t config;
    config.ledc_channel = LEDC_CHANNEL_0;
    config.ledc_timer = LEDC_TIMER_0;
    config.pin_d0 = Y2_GPIO_NUM;
    config.pin_d1 = Y3_GPIO_NUM;
    config.pin_d2 = Y4_GPIO_NUM;
    config.pin_d3 = Y5_GPIO_NUM;
    config.pin_d4 = Y6_GPIO_NUM;
    config.pin_d5 = Y7_GPIO_NUM;
    config.pin_d6 = Y8_GPIO_NUM;
    config.pin_d7 = Y9_GPIO_NUM;
    config.pin_xclk = XCLK_GPIO_NUM;
    config.pin_pclk = PCLK_GPIO_NUM;
    config.pin_vsync = VSYNC_GPIO_NUM;
    config.pin_href = HREF_GPIO_NUM;
    config.pin_sccb_sda = SIOD_GPIO_NUM;
    config.pin_sccb_scl = SIOC_GPIO_NUM;
    config.pin_pwdn = PWDN_GPIO_NUM;
    config.pin_reset = RESET_GPIO_NUM;
    config.xclk_freq_hz = 20*1000*1000;
    config.pixel_format = CAMERA_PIXEL_FORMAT;
    config.jpeg_quality = CAMERA_QUALITY;  // 0-63, lower is better quality
    config.frame_size = _resolution;

    // PSRAM configuration
    if (psramFound()) {
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_LATEST;
        config.fb_count = 1;
    } else {
        config.fb_location = CAMERA_FB_IN_DRAM;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
        config.fb_count = 1;
    }

    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        ESP_LOGE(TAG, "Camera init failed with error 0x%x (%s)", err, esp_err_to_name(err));
        return false;
    }

    _initialized = true;
    return true;
    #endif
}

camera_fb_t* Camera::captureFrame(bool raw) {
    if (!_initialized) {
        return nullptr;
    }

    camera_fb_t* fb = esp_camera_fb_get();

    if (!fb) {
        return fb;
    }

    if (raw || fb->format == PIXFORMAT_JPEG) {
        return fb;
    }

    size_t fb_len = 0;
    uint8_t* fb_buf = NULL;
    esp_err_t ret = frame2jpg(fb, 90, &fb_buf, &fb_len) ? ESP_OK : ESP_FAIL;
    if (ret != ESP_OK) {
        ESP_LOGE(TAG, "convert frame to jpg failed: %s", esp_err_to_name(ret));
        return fb;
    }

    // Check if the JPEG buffer is smaller than the original buffer
    if (fb_len <= fb->len) {
        // Safe to copy - JPEG data fits in the original buffer
        memcpy(fb->buf, fb_buf, fb_len);
        fb->len = fb_len;
        fb->format = PIXFORMAT_JPEG;
        free(fb_buf);
        return fb;
    } else {
        // JPEG is larger than original - this shouldn't happen often with quality 80
        // but let's handle it safely by returning the original frame
        ESP_LOGW(TAG, "JPEG conversion resulted in larger size (%zu > %zu), returning original frame", fb_len, fb->len);
        free(fb_buf);
        return fb;
    }
}

void Camera::returnFrame(camera_fb_t* fb) {
    if (fb) {
        esp_camera_fb_return(fb);
    }
}

void Camera::setResolution(framesize_t resolution) {
    if (!_initialized) {
        _resolution = resolution;
        return;
    }

    sensor_t* sensor = esp_camera_sensor_get();
    if (sensor) {
        sensor->set_framesize(sensor, resolution);
        _resolution = resolution;
    }
}

framesize_t Camera::getResolution() const {
    return _resolution;
}

void Camera::adjustSettings(int brightness, int contrast, int saturation) {
    sensor_t * s = esp_camera_sensor_get();
    if (s) {
        s->set_brightness(s, brightness);
        s->set_contrast(s, contrast);
        s->set_saturation(s, saturation);
    }
}

} // namespace Sensors
