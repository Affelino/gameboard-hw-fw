# Hardware Designs

This directory contains physical design files for different versions of the chessboard hardware.

Each subdirectory includes the schematic and layout (initially as PDF), plus any relevant notes or variations. New versions may differ in size, component placement, LED type, or connector layout — see the individual folders for details.

Note!!! Components on the PDF schema are not the same I ended up using. At least the hall sensor isn't the type I have verified to work. I'll update the pictures but feel free to ping me if it seems to take too long 😉

---

## Controller Requirements

Although example builds (and photos) often show the use of an **ESP-01** module, **this is not a requirement**.

The only hardware features needed by the controller are:

- **I²C support** (for scanning the board matrix via IO expanders)
- **One free GPIO** to drive the LED chain (e.g., WS2812/NeoPixel)

ESP-01 has been a good and affordable starting point for early prototypes, but other options may be more suitable depending on the application.

---

## Alternative Microcontroller Options

- **ESP32 (with Bluetooth)**  
  Useful when connecting the board directly to a mobile device without Wi-Fi.

- **ESP32-S3 (with display and touch support)**  
  Allows adding a user interface directly to the board — e.g., for showing clocks, move lists, menus, or promotion choices.

- **Other MCUs**  
  Any microcontroller with I²C and LED control capability can be adapted to run the firmware.

The protocol itself is agnostic to the controller used — it only defines how the board communicates over a simple serial link.

---

## License

Licensed under [CC BY 4.0](https://creativecommons.org/licenses/by/4.0/)  
See the root-level `LICENSE` file for details.
