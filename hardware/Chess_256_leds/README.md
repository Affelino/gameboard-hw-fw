# Hardware: Full-LED Chessboard (256 LEDs)

This directory contains the hardware documentation for an alternative board design that uses LED strips:

- **4 addressable RGB LEDs per square** (total **256 LEDs**)
- **Magnetic piece detection** using Hall effect sensors
- Designed for microcontrollers supporting large LED arrays (e.g. **ESP8266 / ESP32**)

### Features

- High-visibility indication using 4 LEDs per square
- LED layout driven by **NeoPixel-style LED strips**
- More flexibility for animations, effects, or directional guidance
- Hall sensor matrix remains the same as in [Basic Board](../hw_board_basic_64led)

### Supported Protocol Levels

This board supports all features of the  
[Gameboard UI Protocol](https://github.com/Affelino/gameboard-ui-protocol), and is  
especially well-suited for firmware targeting **levels 0.1 and 1** due to the visual clarity of the LEDs.

It can also support **level X** (internal validation logic), depending on firmware.

**⚠️ Level Y (embedded engine)** may require a more capable MCU (e.g. ESP32 with additional memory).

### Notes

- The LED strip layout defines physical board dimensions more strictly than single-LED designs.
- LED mapping logic is more complex, but enables advanced UX features.

---

Licensed under the Creative Commons Attribution 4.0 International License (CC BY 4.0).  
See the root-level [LICENSE](../../LICENSE) file for full details.
