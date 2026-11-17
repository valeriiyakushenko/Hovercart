# Hovercart

<!-- Cover image: add a hero photo here to show up as the repo cover on GitHub -->
<!-- ![Hovercart cover](docs/cover.jpg) -->

Radio-controlled car built from a hoverboard — four BLDC motors, ESP32, and a PS5 DualSense controller over Bluetooth.

[![PlatformIO](https://img.shields.io/badge/PlatformIO-3.5.0-208844?style=for-the-badge&logo=platformio)](https://platformio.org/)
[![ESP32](https://img.shields.io/badge/ESP32-Espressif-000000?style=for-the-badge&logo=espressif)](https://www.espressif.com/)
[![Arduino](https://img.shields.io/badge/Arduino-1.8.19-00979D?style=for-the-badge&logo=arduino)](https://www.arduino.cc/)
[![Bluetooth](https://img.shields.io/badge/Bluetooth-LE-0082C9?style=for-the-badge&logo=bluetooth)](https://en.wikipedia.org/wiki/Bluetooth_Low_Energy)
[![License: MIT](https://img.shields.io/badge/License-MIT-yellow?style=for-the-badge)](LICENSE)

## Hardware

- **MCU:** ESP32 (ESP-WROOM-32)
- **Motors:** 4× hoverboard BLDC motors, each driven by a ZS-X11H v2 driver
- **Steering:** SG90-style servo on the front axle
- **Controller:** PS5 DualSense via Bluetooth LE

Each motor driver takes PWM (speed), DIR (direction), BRAKE (brake), and sends back SPEED (Hall sensor pulses). The ESP32 monitors those pulses to detect when wheels actually stop before switching direction.

## Gallery

[![YouTube video](https://img.shields.io/badge/YouTube-Watch%20on%20YouTube-red?style=for-the-badge&logo=youtube)](YOUR_YOUTUBE_LINK_HERE)

<!-- Add photos below, e.g.:
![Hovercart build](docs/build.jpg)
![Hovercart in action](docs/action.jpg)
-->

<!-- Wiring diagram:
![Wiring diagram](docs/wiring.png)
-->

## Wiring

| Motor | PWM | DIR | BRAKE | SPEED | Inverted |
|-------|-----|-----|-------|-------|----------|
| Front Left  | 25 | 17 | 16 | 34 | Yes |
| Front Right | 26 | 13 | 4  | 35 | No  |
| Back Left   | 27 | 19 | 18 | 36 | Yes |
| Back Right  | 33 | 32 | 23 | 39 | No  |

- Servo: GPIO 14
- Status LED: GPIO 2
- DIR forward = HIGH, BRAKE active = LOW
- Note: ESP32 outputs 3.3 V logic; the ZS-X11H datasheet expects 5 V PWM. Works fine in practice, but keep it in mind.

Full pin definitions are in `src/wiring.h`.

## Controls

| Input | Action |
|-------|--------|
| R2 | Forward throttle |
| L2 | Reverse throttle |
| R1 | Brake (all wheels) |
| L1 | Front brake (drift mode only — burnout) |
| Left stick X | Steering |

**No controller connected?** The car brakes automatically and the status LED blinks.

## Modes

Hold both analog sticks (L3 + R3) for 3 seconds with zero throttle to enter the mode menu. Use the D-pad left/right to cycle, Cross to confirm.

| Mode | Lightbar | Power limit | Drive |
|------|----------|-------------|-------|
| Standard | Green | 25% | All 4 wheels |
| Performance | Purple | 100% | All 4 wheels |
| Drift | Red | 100% | Rear only |

In drift mode, R1 brakes only the rear wheels (the front keeps spinning for a burnout). L1 brakes only the front wheels while the rear keeps pulling.

## Build

Requires [PlatformIO](https://platformio.org/).

```bash
# Install dependencies
pio lib install "madhephaestus/ESP32Servo@^3.0.5"

# Set your controller's MAC address (find it in your phone/PC Bluetooth settings)
# Edit platformio.ini and change the CONTROLLER_MAC flag

# Upload
pio run --target upload --upload-port /dev/ttyUSB0

# Monitor serial output
pio device monitor --baud 115200
```

The controller MAC address is set via `build_flags` in `platformio.ini`. Change `CONTROLLER_MAC` to match your DualSense.

## How it works

- **Motors** use the ESP32's LEDC peripheral at 15 kHz, 10-bit resolution. Throttle follows a quadratic curve (`duty = (v/255)² × 1023`) so low-speed control is smooth.
- **Direction changes** are safe: power off → wait for all wheels to stop (no SPEED pulses for 200 ms) → 100 ms settle → flip DIR → power on.
- **Steering** maps the stick X-axis (-128 to 127) to servo angles 5°–175° with a 2.5% deadzone in the center.
- **Controller communication** parses both PS5 report types (0x01 and 0x31) because the gamepad switches formats after the first LED packet. The lightbar is controlled directly via the L2CAP interrupt channel — the stock library's `setLed` sends a PS4-format packet that DualSense ignores.

## File structure

```
Hovercart/
├── src/
│   ├── main.cpp      — Main loop, mode menu, setup
│   ├── motors.cpp    — LEDC PWM, direction switching, wheel stop detection
│   ├── motors.h
│   ├── steering.cpp  — Servo control with deadzone
│   ├── steering.h
│   ├── controller.cpp — PS5 DualSense Bluetooth wrapper, lightbar control
│   ├── controller.h
│   └── wiring.h      — Pin definitions for all motors, servo, LED
├── docs/             — Photos, wiring diagram, video
└── hardware/         — 3D models, PCB files, mechanical drawings
```

## Resources

- [ZS-X11H v2 Datasheet](https://www.aliexpress.com/item/1005005869284018.html) — motor driver module
- [ps5-esp32 library](https://github.com/rodneybakiskan/ps5-esp32) — PS5 controller Bluetooth library
- [ESP32Servo](https://github.com/madhephaestus/ESP32Servo) — servo library for ESP32
- [PlatformIO](https://platformio.org/) — build system
- [ESP32 LEDC](https://docs.platformio.org/en/latest/platforms/espressif32.html) — LEDC PWM peripheral docs

## License

MIT
