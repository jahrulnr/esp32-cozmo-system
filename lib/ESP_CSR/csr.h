/*
 * SPDX-FileCopyrightText: 2015-2022 Espressif Systems (Shanghai) CO LTD
 *
 * SPDX-License-Identifier: Unlicense OR CC0-1.0
 */

#pragma once
#include "sdkconfig.h"
#include <Arduino.h>
#if CONFIG_IDF_TARGET_ESP32S3 && (CONFIG_USE_WAKENET || CONFIG_USE_MULTINET)

#include "driver/i2s_types.h"
#include "esp32-hal-sr.h"
#include "esp_err.h"

#define SR_CMD_STR_LEN_MAX     64
#define SR_CMD_PHONEME_LEN_MAX 64

typedef struct csr_cmd_t {
  int command_id;
  char str[SR_CMD_STR_LEN_MAX];
  char phoneme[SR_CMD_PHONEME_LEN_MAX];
} csr_cmd_t;

namespace SR {

	esp_err_t sr_start(
		sr_fill_cb fill_cb, void *fill_cb_arg, sr_channels_t rx_chan, sr_mode_t mode, const csr_cmd_t *sr_commands, size_t cmd_number, sr_event_cb cb, void *cb_arg
	);
	esp_err_t sr_stop(void);
	esp_err_t sr_pause(void);
	esp_err_t sr_resume(void);
	esp_err_t sr_set_mode(sr_mode_t mode);

}

#endif  // CONFIG_IDF_TARGET_ESP32S3
