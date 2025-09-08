## GPIO Pin Mapping

### **Display & I2C Communication**
- **GPIO 3**: Screen SCL (I2C Clock) + Orientation Sensor SCL (**USED**)
- **GPIO 46**: Screen SDA (I2C Data) + Orientation Sensor SDA (**USED**)

### **Audio System**
- **GPIO 21**: Microphone WS (I2S Word Select) (**USED**)
- **GPIO 47**: Microphone SCK (I2S Serial Clock) (**USED**)
- **GPIO 14**: Microphone DIN (I2S Data Input) (**USED**)
- **GPIO 42**: Speaker BCLK (I2S Bit Clock) (**USED**)
- **GPIO 2**: Speaker WCLK (I2S Word Clock/LR Clock) (**USED**)
- **GPIO 41**: Speaker DATA (I2S Data Output to MAX98357) (**USED**)

### **Servo Motors**
- **GPIO 19**: Head Servo Control (**USED**)
- **GPIO 20**: Hand Servo Control (**USED**)

### **Ultrasonic Sensor**
- **GPIO 0**: Ultrasonic Trigger Pin (**USED**)
- **GPIO 45**: Ultrasonic Echo Pin (**USED**)

### **SD Card (SD_MMC Interface)**
- **GPIO 39**: SD_MMC Clock (**USED**)
- **GPIO 38**: SD_MMC Command (**USED**)
- **GPIO 40**: SD_MMC Data 0 (**USED**)

### **Battery Monitoring**
- **GPIO 1**: Battery Voltage ADC (via 100kΩ voltage divider) (**USED**)

### **Motors (via IOExtern/PCF8575 I2C Expander)**
- **Expander Pin 1**: Right Motor Pin 2 (**USED**)
- **Expander Pin 2**: Right Motor Pin 1 (**USED**)
- **Expander Pin 3**: Left Motor Pin 1 (**USED**)
- **Expander Pin 4**: Left Motor Pin 2 (**USED**)

### **Cliff Detection (via IOExtern/PCF8575 I2C Expander)**
- **Expander Pin 5**: Right Cliff Detector (**NOT USED**, see CLIFF_DETECTOR_ENABLED)
- **Expander Pin 6**: Left Cliff Detector (**NOT USED**, see CLIFF_DETECTOR_ENABLED)

### **Unused/Optional Pins**

#### ESP32-S3 Unused GPIO Pins (0-47)

The following pins are currently **NOT USED** in this system configuration:

- **GPIO 4** (Camera: FREENOVE)
- **GPIO 5** (Camera: AI Thinker, FREENOVE)
- **GPIO 6** (Camera: FREENOVE)
- **GPIO 7** (Camera: FREENOVE)
- **GPIO 8** (Camera: FREENOVE)
- **GPIO 9** (Camera: FREENOVE)
- **GPIO 10** (Camera: XIAO, FREENOVE)
- **GPIO 11** (Camera: XIAO, FREENOVE)
- **GPIO 12** (Camera: XIAO, FREENOVE)
- **GPIO 13** (Camera: XIAO, FREENOVE)
- **GPIO 15** (Camera: XIAO, FREENOVE)
- **GPIO 16** (Camera: XIAO, FREENOVE)
- **GPIO 17** (Camera: XIAO, FREENOVE)
- **GPIO 18** (Camera: AI Thinker, XIAO, FREENOVE)
- **GPIO 22** (Camera: AI Thinker)
- **GPIO 23** (Camera: AI Thinker)
- **GPIO 24**
- **GPIO 25** (Camera: AI Thinker)
- **GPIO 26** (Camera: AI Thinker)
- **GPIO 27** (Camera: AI Thinker)
- **GPIO 28**
- **GPIO 29**
- **GPIO 30**
- **GPIO 31**
- **GPIO 32** (Camera: AI Thinker)
- **GPIO 33**
- **GPIO 34** (Camera: AI Thinker)
- **GPIO 35** (Camera: AI Thinker)
- **GPIO 36** (Camera: AI Thinker)
- **GPIO 37**
- **GPIO 43**
- **GPIO 44**
- **GPIO 48** (Camera: XIAO)
- **GPIO -1**: Microphone gain/AR pins (NOT USED)

#### Camera Pins (from CameraConfig.h)

The following pins are used by camera modules if enabled. If CAMERA_ENABLED is false, these pins are currently **NOT USED**:

- **AI Thinker**: 0, 5, 18, 19, 21, 22, 23, 25, 26, 27, 32, 34, 35, 36, 39
- **XIAO ESP32S3**: 10, 11, 12, 13, 14, 15, 16, 17, 18, 38, 39, 40, 47, 48
- **FREENOVE ESP32S3 CAM**: 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 15, 16, 17, 18

> Note: Camera pins are only active if CAMERA_ENABLED and the corresponding model macro is defined in Config.h. Otherwise, they are considered unused.

## 📝 Notes:
- **I2C Bus Shared**: Display and orientation sensor share the same I2C bus (GPIO 3/46)
- **IOExtern Dependency**: Motors require PCF8575 I2C expander; Cliff detectors are currently disabled
- **I2S Audio**: Separate I2S buses for microphone input and speaker output
- **Battery Monitoring**: Uses 12-bit ADC with 100kΩ voltage divider (R1=R2) + ceramic capacitor for stability
- **Feature Flags**: All hardware controlled by `*_ENABLED` macros in Config.h
- **Camera Pins**: See CameraConfig.h for camera-specific pin usage. All other camera pins are NOT USED unless camera is enabled and model matches.