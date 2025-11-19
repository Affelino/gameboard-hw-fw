# Firmware: Level 0.1 Implementation

This firmware implements the **Level 0.1** functionality of the  
[Gameboard UI Protocol](https://github.com/Affelino/gameboard-ui-protocol).

## 🔧 Supported Features (Level 0.1)

- Detects pick-up (`-e2`) and put-down (`+e4`) events via sensor matrix
- Sends basic move messages to an external host or engine
- Receives LED instructions to guide user actions
- Verifies that the user follows the guided moves, indicates wrong moves and tries to indicate how to correct
- Controls RGB LEDs (1 per square or 4 per square depending on hardware)
- Does **not** validate user input or track game state

This is a deliberately simple firmware designed for basic board ↔️ engine communication.  
It is fully usable for guided move playback and manual move input with minimal logic.

---

## 🧱 Hardware Support

This firmware supports two physical board designs:

- **1 LED per square** – 8×8 layout, total 64 RGB LEDs
- **4 LEDs per square** – 8×8 layout using LED strips, total 256 RGB LEDs

Both variants share the same core firmware. The active board type is selected at compile time using a `#define`.

Target platform:  
- **ESP8266 (ESP-01)** with I²C sensor matrix and NeoPixel-compatible LED output

Other MCUs can be used as long as they support:
- I²C (for input)
- One GPIO for LED output (NeoPixel / WS2812)

---

## 🚧 Status

This version is still being finalized. Comments are pretty much non-existent, but hopefully the code is readable enough as it is. If something looks weird (wouldn't surprise me), let me know and I'll see if I can open it up.

---

## 📄 License

Licensed under the  
[Creative Commons Attribution 4.0 International License (CC BY 4.0)](https://creativecommons.org/licenses/by/4.0/).  
See the root-level [LICENSE](../../LICENSE) file for full details.
