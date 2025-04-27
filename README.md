# Arduino ESP32: COM2BLE/KM

The bridge device that receives keyboard codes and mouse events over a serial (COM) port and emits them as a BLE HID device.

## Features

- Receives key codes and mouse events over a serial connection
- Transmits as BLE HID (keyboard and mouse)
- Supports M5Atom and other ESP32-based devices
- Lightweight and low-latency operation
- Easy integration with embedded systems or custom controllers

## Typical Use Cases

- Wireless conversion of wired input devices
- Custom controller projects (e.g., arcade sticks, accessibility devices)
- Industrial systems needing Bluetooth HID output

## Hardware Requirements

- M5Atom (Lite, Matrix, or Echo)
- Any ESP32 board with BLE capability
- Serial input interface (UART)
