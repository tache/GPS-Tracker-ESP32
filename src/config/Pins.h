#pragma once
// GC9A01A on ESP32-C3 Super Mini (SPI, write-only, no backlight pin).
// VCC -> 3V3 (NOT 5V), GND -> GND.
namespace pins {
constexpr int kDisplayRst  = 0;   // RST
constexpr int kDisplayCs   = 1;   // CS
constexpr int kDisplayDc   = 10;  // DC
constexpr int kDisplayMosi = 3;   // SDA (MOSI)
constexpr int kDisplaySclk = 4;   // SCL (SCLK)
}  // namespace pins
