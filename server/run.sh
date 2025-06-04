#!/bin/bash

# Build the Go application
echo "Building Cozmo Web Server..."
go build -o cozmo-web-server ./cmd/main.go

# Check if build was successful
if [ $? -ne 0 ]; then
    echo "Build failed!"
    exit 1
fi

# Run the server
echo "Starting Cozmo Web Server..."
./cozmo-web-server
