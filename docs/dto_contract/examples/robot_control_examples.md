# Robot Control DTO Examples

## Motor Command

```json
{
  "version": "1.0",
  "type": "robot_motor",
  "data": {
    "left": 75,
    "right": 75,
    "duration": 2000
  }
}
```

## Servo Command

```json
{
  "version": "1.0",
  "type": "robot_servo",
  "data": {
    "id": "head_y",
    "angle": 45,
    "speed": 100
  }
}
```

## Joystick Command (Motor Control)

```json
{
  "version": "1.0",
  "type": "robot_joystick",
  "data": {
    "type": "motor",
    "x": 25,
    "y": -50
  }
}
```

## Joystick Command (Servo Control)

```json
{
  "version": "1.0",
  "type": "robot_joystick",
  "data": {
    "type": "servo",
    "x": 15,
    "y": 75
  }
}
```

## Robot Preset Command

```json
{
  "version": "1.0",
  "type": "robot_preset",
  "data": {
    "preset": "forward",
    "duration": 3000,
    "speed": 75
  }
}
```

## Emergency Stop Command

```json
{
  "version": "1.0",
  "type": "robot_preset",
  "data": {
    "preset": "stop"
  }
}
```
