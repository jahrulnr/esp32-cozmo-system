#pragma once

#include "setup/setup.h"
#include "tasks/register.h"
#include <esp32-hal-sr.h>

esp_err_t mic_fill_callback(void *arg, void *out, size_t len, size_t *bytes_read, uint32_t timeout_ms);
void sr_event_callback(void *arg, sr_event_t event, int command_id, int phrase_id);

void picotts_output_callback(int16_t *samples, unsigned count);
void picotts_error_callback(void);
void picotts_idle_callback(void);

void weatherCallback(const Services::WeatherService::WeatherData &data, bool success);
void batteryCallback(void* arg);
void callbackNotePlayer(void* data);