#!/bin/bash

xtensa-esp32-elf-addr2line -e ./.pio/build/esp32s3dev/firmware.elf "$@"