#!/bin/bash

ffmpeg -i "$1" -ar 16000 -ac 2 "$2"
