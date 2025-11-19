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

This version will not be actively developed, because in practice it is not very useful. It verifies that the user picks up the correct piece and places it on the correct square — but that’s about it. There are many corner cases that would need additional logic for consistent behavior.

For example, when a capture occurs, the board can indicate which piece should move and where it should go, but how should it react to the removal of the captured piece? Handling that cleanly requires more logic, and only a simple demonstration is included in this version.

If you intend to use the board as a UI device for an existing chess application, the Level 0 implementation is the recommended choice. Since the mobile chess app already knows the full game state and can validate moves, it makes sense to let the app handle all move legality checks instead of duplicating that logic on the board.

---

## 📄 License

Licensed under the  
[Creative Commons Attribution 4.0 International License (CC BY 4.0)](https://creativecommons.org/licenses/by/4.0/).  
See the root-level [LICENSE](../../LICENSE) file for full details.
