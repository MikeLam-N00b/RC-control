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

Files and images
- `esp32transmitter.ino` — transmitter sketch (read ADC, send ESP‑NOW packets).
- `transmitter electrical/` — images for the schematic, PCB top layout, and 3D render. Expected filenames (already added): `schematic.png`, `pcb_top.png`, `3d_view.png`.

Usage
1. Open `esp32transmitter.ino` in Arduino IDE or PlatformIO.
2. Configure target board to an ESP32 variant and set correct ADC pins matching the schematic.
3. Ensure the receiver ESP32 MAC address is set in the transmitter code (or use broadcast if appropriate).
4. Upload `esp32receiver` to the car’s ESP32 and power the system. Test by moving pots and observing the servo/motor response.

Safety notes
- Double-check grounding: both transmitter and receiver must share a common ground reference when required by signal wiring.
- Keep high-current motor/servo wiring separate from sensitive analog traces to avoid noise coupling.

License & contact
- Add a `LICENSE` file if you want to apply a specific open-source license (MIT is common for small projects).
- Questions or corrections: open an issue on the repo or contact the author (Lam Kim Bao).

---
Additional help
If you want, I can:
- Add wiring diagrams or a more detailed section listing recommended resistor/capacitor values used in the RC filters.
- Add a `LICENSE` file (MIT/Apache-2.0) and expand the `transmitter electrical/README.md` with image captions.
Tell me which and I’ll update the repo.
