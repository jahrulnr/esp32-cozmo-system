#!/bin/bash

xtensa-esp32-elf-addr2line -e ./.pio/build/esp32cam/firmware.elf "$@"