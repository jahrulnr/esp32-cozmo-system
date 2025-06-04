#include "Camera.h"
#include "../../include/Config.h"

namespace Sensors {

Camera::Camera() : _resolution(CAMERA_FRAME_SIZE), _initialized(false), _streamingInterval(200) {
}

Camera::~Camera() {
    if (_initialized) {
        esp_camera_deinit();
    }
}

bool Camera::init() {
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
    config.xclk_freq_hz = 20000000;
    config.pixel_format = PIXFORMAT_JPEG;
    config.jpeg_quality = CAMERA_QUALITY;  // 0-63, lower is better quality
    
    // PSRAM configuration
    if (psramFound()) {
        config.frame_size = _resolution;
        config.fb_location = CAMERA_FB_IN_PSRAM;
        config.grab_mode = CAMERA_GRAB_LATEST;
        config.fb_count = 2;
    } else {
        config.frame_size = FRAMESIZE_SVGA;
        config.fb_location = CAMERA_FB_IN_DRAM;
        config.grab_mode = CAMERA_GRAB_WHEN_EMPTY;
        config.fb_count = 1;
    }
    
    esp_err_t err = esp_camera_init(&config);
    if (err != ESP_OK) {
        Serial.printf("Camera init failed with error 0x%x\n", err);
        return false;
    }
    
    _initialized = true;
    return true;
}

camera_fb_t* Camera::captureFrame() {
    if (!_initialized) {
        return nullptr;
    }
    return esp_camera_fb_get();
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

uint32_t Camera::getStreamingInterval() const {
    return _streamingInterval;
}

void Camera::setStreamingInterval(uint32_t interval) {
    _streamingInterval = interval;
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
