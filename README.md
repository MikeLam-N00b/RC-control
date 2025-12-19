# ESP32 Transmitter

This repository holds the transmitter-side Arduino sketch and supporting files for an ESP‑NOW based RC system.

Summary
- **Transmitter:** `esp32transmitter.ino` — ESP32 sketch that reads position from variable resistors (trim/potentiometers) and sends control packets using ESP‑NOW.
- **Receiver:** `esp32receiver` — (separate sketch) runs on the ESP32 mounted in the RC car to receive commands and drive servos/motor drivers.
- **Electrical assets:** `transmitter electrical/` — schematic, PCB layout and 3D render images for the transmitter hardware.

How it works (ESP‑NOW)
- ESP‑NOW is a low‑latency, connectionless protocol supported by the ESP32 family. It allows one ESP32 to broadcast small data packets to another without forming a Wi‑Fi AP/STA connection.
- In this project the transmitter reads analog positions (variable resistors) and encodes them into compact control messages. The transmitter then sends those messages via ESP‑NOW to the receiver ESP32 on the car.
- The receiver listens for packets, decodes the values, and uses them to command the servo(s) and motor driver(s).

Electrical design notes (RC filter and signal flow)
- Purpose: The provided schematic shows how the variable resistors (pots) are connected to the transmitter’s ADC inputs and how simple RC filtering and buffering are applied before sending values.
- Variable resistor inputs: Each control axis (e.g., steering trim, throttle) is read by an ADC pin on the ESP32. The schematic includes series resistors and small capacitors to form low‑pass RC filters that help stabilize ADC readings by smoothing noise and switch bounce from mechanical pots.
- RC filter basics: A series resistor (R) with a capacitor (C) to ground forms a low‑pass filter. Typical values used for human-operated pots are in the low‑kΩ and tens to hundreds of nF range, balancing response time and noise rejection.
- Power and servo/motor considerations: Servos and DC motors draw significant current and can inject noise onto the supply lines. The transmitter’s schematic separates the control electronics (ESP32, ADC filtering) from heavy loads and includes decoupling capacitors and a regulated 3.3V source for the ESP32. The receiver (on the car) should supply servos from a stable power rail (separate from the logic 3.3V when possible) and include flyback protection and filtering on motor drivers.

# ESP32 ESP‑NOW RC Transmitter

[![License](https://img.shields.io/badge/License-MIT-green.svg)](./LICENSE)

**Author:** Lâm Kim Bảo  
**Email:** 731819lamkimbao@gmail.com  
**Date:** 20/12/2025

## Table of Contents
- [Overview](#overview)
- [Repository Contents](#repository-contents)
- [Hardware Requirements](#hardware-requirements)
- [Pinout](#pinout)
- [Control Messages](#control-messages)
- [Setup and Upload](#setup-and-upload)
- [Testing](#testing)
- [MISRA and Security Notes](#misra-and-security-notes)
- [License](#license)
- [Contact / Contributing](#contact--contributing)

---

## Overview

This repository contains the transmitter-side code and supporting hardware images for an ESP‑NOW based RC system. The transmitter reads analog inputs (variable resistors/potentiometers) and sends compact control packets to a receiver ESP32 mounted on an RC car. The receiver decodes the packets and drives servos and motor controllers accordingly.

ESP‑NOW provides a low-latency, connectionless link between ESP32 devices and is well suited for remote control applications where minimal overhead and fast updates are required.

## Repository Contents

- `esp32transmitter.ino` — main Arduino sketch (transmitter).
- `esp32receiver` — (separate sketch) receiver for the car — included in the repo root or sibling repo.
- `transmitter electrical/` — schematic and PCB images: `schematic.png`, `pcb_top.png`, `3d_view.png`.
- `README.md` — this file.

## Hardware Requirements

- ESP32 development board (supported by Arduino core)
- 3× or more potentiometers (variable resistors) for control axes (e.g., steering, throttle, trim)
- Power supply for transmitter (regulated 3.3V for ESP32)
- Receiver side: ESP32, servo motors, motor driver (external power recommended for motors/servos)

**Important:** Servos and motors must be powered from a separate high-current supply; share ground with ESP32.

## Pinout

Map the ADC pins in `esp32transmitter.ino` to the potentiometers used on the transmitter PCB.

Example (adjust to match schematic):

| Function | ESP32 ADC Pin |
|----------|---------------:|
| Throttle | ADC1_0 / GPIO36 |
| Steering | ADC1_3 / GPIO39 |
| Trim     | ADC1_4 / GPIO32 |

Refer to the schematic image in `transmitter electrical/schematic.png` for exact PCB pin labels.

## Control Messages

The transmitter encodes potentiometer positions into compact ESP‑NOW packets. Typical message layout (example):

- 1 byte: Packet ID / sequence
- 1 byte: Channel flags
- 3× 2 bytes: Signed 16-bit positions for Throttle, Steering, Trim

The receiver unpacks these values and maps them to servo angles or motor speed after applying calibration and dead‑band.

## Setup and Upload

### Software Requirements

- Arduino IDE or `arduino-cli`  
- ESP32 Arduino core  
- Optional: `ESP32Servo` or servo helper libraries on the receiver

### Using Arduino CLI (PowerShell)
```powershell
# install/verify core
arduino-cli core update-index
arduino-cli core install esp32:esp32

# (optional) install libraries
arduino-cli lib install "ESP32Servo"

# compile and upload transmitter (adjust port and fqbn)
arduino-cli compile --fqbn esp32:esp32:esp32 .\esp32transmitter.ino
arduino-cli upload -p COM3 --fqbn esp32:esp32:esp32 .\esp32transmitter.ino
```

Notes:
- Configure the correct ESP32 board and COM port.  
- Set the receiver MAC address in the transmitter sketch or use broadcast pairing.

## Testing

1. Power the transmitter and receiver (ensure common ground if required by wiring).
2. Open Serial Monitor (115200 baud) on both devices for debug output.
3. Move the potentiometers on the transmitter and verify received values on the receiver log.
4. Gradually test servo and motor commands with motors disconnected first, then with a powered motor/servo after confirming correct control signals.

## MISRA and Security Notes

This project follows safer C/C++ idioms where practical: fixed-width integer types, explicit casts, and minimized global visibility. Full MISRA compliance is not possible with the Arduino runtime, but the code aims to be clearer and easier to analyze.

Security:
- ESP‑NOW packets are not encrypted by default. If security is required, enable ESP‑NOW encryption or use a secure pairing/key mechanism.

## License

This project is provided under the MIT License. Add a `LICENSE` file in the repo root to apply.

## Contact / Contributing

Author: Lâm Kim Bảo  
Email: 731819lamkimbao@gmail.com

Contributions, bug reports and feature requests are welcome — open an issue or submit a pull request.

---

If you'd like I can also:
- Add a `LICENSE` file (MIT) and commit it.
- Add image captions in `transmitter electrical/README.md` describing each image.
- Add a short `USAGE.md` with wiring diagrams and recommended R/C filter component values.
