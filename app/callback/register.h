#pragma once

#include "setup/setup.h"
#include "tasks/register.h"
#include <esp32-hal-sr.h>

esp_err_t sr_fill_callback(void *arg, void *out, size_t len, size_t *bytes_read, uint32_t timeout_ms);
void sr_event_callback(void *arg, sr_event_t event, int command_id, int phrase_id);