# Hardware: Basic Chessboard (64 LEDs)

This directory contains the hardware documentation for a minimal electronic chessboard:

- **1 RGB LED per square** (total 64 LEDs)
- **Magnetic piece detection** using Hall effect sensors
- **MCP23017 I/O expander** for scanning 8×8 matrix
- Designed for microcontrollers like **ESP-01** or **ESP8266 D1 mini**

### Features

- Simple square-level detection (occupied / empty)
- LED guidance for user interaction (move targets, warnings, etc.)
- Compact board design with no piece-type recognition

### Supported Protocol Levels

This board is suitable for firmware implementing **protocol levels 0–1** of the  
[Gameboard UI Protocol](https://github.com/Affelino/gameboard-ui-protocol).

It can also support advanced modes up to **Level X** (e.g. full UCI validation),  
as long as the MCU and firmware can handle the logic.

**⚠️ Level Y (embedded chess engine)** is *not feasible* on this board due to limited processing power.

### Notes

- The board layout assumes one LED per square, simplifying wiring and code logic.
- This version is ideal for testing, tinkering, and firmware development.

---

Licensed under the Creative Commons Attribution 4.0 International License (CC BY 4.0).  
See the root-level [LICENSE](../../LICENSE) file for full details.
