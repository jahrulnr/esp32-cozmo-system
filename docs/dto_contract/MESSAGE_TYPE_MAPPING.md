# DTO Message Type Mapping

This document maps the message types in the DTO contract documentation to their implementation in the codebase. It helps identify inconsistencies and provides guidance on standardization.

## Message Type Mapping Table

| Category | Documented Type | Implemented Type | Direction | Purpose | Status |
|----------|----------------|-----------------|-----------|---------|--------|
| **Authentication** |
| Auth | `auth_login` | `login` | Client → Server | Authentication request | ⚠️ Mismatch |
| Auth | `auth_login_response` | `login_response` | Server → Client | Authentication result | ⚠️ Mismatch |
| Auth | `auth_logout` | Not found | Client → Server | Logout request | ❓ Not implemented |
| **Motor Control** |
| Robot | `robot_motor` | `motor_command` | Client → Server | Motor control | ⚠️ Mismatch |
| Robot | `robot_servo` | `servo_update` | Client → Server | Servo control | ⚠️ Mismatch |
| Robot | N/A | `joystick_update` | Client → Server | Joystick control | ❓ Not documented |
| **Camera** |
| Camera | `camera_stream` | `camera_command` | Client → Server | Camera stream control | ⚠️ Mismatch |
| Camera | `camera_frame` | `camera_frame` | Server → Client | Camera frame data | ✅ Match |
| **Sensors** |
| Sensor | `sensor_data` | `sensor_data` | Server → Client | Sensor readings | ✅ Match |
| Sensor | `sensor_request` | Not found | Client → Server | Request sensor data | ❓ Not implemented |
| **System** |
| System | `system_status` | `system_status` | Server → Client | System information | ✅ Match |
| System | `system_command` | Not found | Client → Server | System commands | ❓ Not implemented |
| **WiFi** |
| WiFi | `wifi_list` | `wifi_list` | Server → Client | Available WiFi networks | ✅ Match |
| WiFi | `wifi_connection` | `wifi_connection` | Server → Client | WiFi connection status | ✅ Match |
| WiFi | Not documented | `connect_wifi` | Client → Server | WiFi connection request | ❓ Not documented |
| **File System** |
| File | `file_list` | `list_files` | Server → Client | Directory listing | ⚠️ Mismatch |
| File | `file_operation` | `file_operation` | Server → Client | File operation result | ✅ Match |
| File | Not documented | `read_file` | Client → Server | Read file request | ❓ Not documented |
| **Logs** |
| Log | Not documented | `log_message` | Server → Client | Single log message | ❓ Not documented |
| Log | Not documented | `batch_log_messages` | Server → Client | Multiple log messages | ❓ Not documented |
| **Errors** |
| Error | `error` | `error` | Server → Client | Error information | ✅ Match |

## Implementation Analysis

The mapping shows several patterns:

1. **Direct matches (✅)**: Some message types are consistent between documentation and implementation.

2. **Naming mismatches (⚠️)**: Some message types follow different naming conventions:
   - Documentation uses categorical prefixes (`auth_login`)
   - Implementation uses direct action names (`login`)

3. **Missing documentation (❓)**: Several implemented message types are not documented.

4. **Not implemented (❓)**: Some documented message types don't appear in the implementation.

## Recommendation for Standardization

Based on the actual implementation, the more practical approach would be to update the documentation to match the implementation rather than changing all the code. The recommended naming convention is:

### Naming Convention

Use descriptive action-based names without category prefixes, but group them in documentation by category:

```
Authentication:
  - login
  - login_response

Motor Control:
  - motor_command
  - servo_update
  - joystick_update

Camera:
  - camera_command
  - camera_frame
```

### Implementation Guidelines

1. **For new message types**:
   - Use descriptive action-based names
   - Document them in the appropriate category
   - Follow the established patterns

2. **For documentation updates**:
   - Remove the category prefixes from message type names
   - Keep the categorical organization for clarity
   - Add all implemented but undocumented message types

## Next Steps

1. Update the DTO contract documentation (README.md and examples) to match the actual implementation
2. Add the missing message types to the documentation
3. Create a complete registry of all message types with examples
4. Implement any documented message types that are missing from the code
5. Add validation for message format and types

This standardization will ensure your DTO contract is accurate, consistent, and provides a reliable foundation for the integration phases in your roadmap.
