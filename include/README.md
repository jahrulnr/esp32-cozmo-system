## GPIO Pin Mapping

### **Display & I2C Communication**
- **GPIO 3**: Screen SDA (I2C Data) + Orientation Sensor SDA
- **GPIO 46**: Screen SCL (I2C Clock) + Orientation Sensor SCL

### **Audio System**
- **GPIO 21**: Microphone WS (I2S Word Select)
- **GPIO 47**: Microphone SCK (I2S Serial Clock)  
- **GPIO 14**: Microphone DIN (I2S Data Input)
- **GPIO 42**: Speaker BCLK (I2S Bit Clock)
- **GPIO 2**: Speaker WCLK (I2S Word Clock/LR Clock)
- **GPIO 41**: Speaker DATA (I2S Data Output to MAX98357)

### **Servo Motors**
- **GPIO 19**: Head Servo Control
- **GPIO 20**: Hand Servo Control

### **Ultrasonic Sensor**
- **GPIO 0**: Ultrasonic Trigger Pin
- **GPIO 45**: Ultrasonic Echo Pin

### **SD Card (SD_MMC Interface)**
- **GPIO 39**: SD_MMC Clock
- **GPIO 38**: SD_MMC Command
- **GPIO 40**: SD_MMC Data 0

### **Motors (via IOExtern/PCF8575 I2C Expander)**
- **Expander Pin 1**: Right Motor Pin 2
- **Expander Pin 2**: Right Motor Pin 1  
- **Expander Pin 3**: Left Motor Pin 1
- **Expander Pin 4**: Left Motor Pin 2

### **Cliff Detection (via IOExtern/PCF8575 I2C Expander)**
- **Expander Pin 5**: Right Cliff Detector
- **Expander Pin 6**: Left Cliff Detector

### **Unused/Optional Pins**
- **GPIO 0**: Also defined for analog microphone (currently disabled)
- **GPIO -1**: Microphone gain/AR pins (not connected)

## 📝 Notes:
- **I2C Bus Shared**: Display and orientation sensor share the same I2C bus (GPIO 3/46)
- **IOExtern Dependency**: Motors and cliff detectors require PCF8575 I2C expander
- **I2S Audio**: Separate I2S buses for microphone input and speaker output
- **Feature Flags**: All hardware controlled by `*_ENABLED` macros in Config.h