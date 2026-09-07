# DL4mk2_reverbcontroller_RPpico

A physical MIDI controller specifically designed for the Line 6 DL4 MkII. It
provides immediate access to hidden reverb parameters, eliminating the need for
complex "ALT + Knob" combinations.

Features

  - Instant Reverb Selection: Switch between all 16 reverb types (including Off)
    via a rotary encoder.
  - Dynamic HUD (Head-Up Display): The OLED pops up the specific parameter name
    and value in real-time only when a knob is adjusted.
  - Context-Aware Labels: Automatically displays the correct "Tweak" parameter
    name (e.g., PreDelay, Mod Depth) based on the currently selected reverb
    model.
  - Smart Mix Display: Shows the Dry/Wet ratio (e.g., D50:W50) and a progress
    bar with a center-line indicator for precise Mix control.
  - Dedicated Oscillation Control: The 4th knob is mapped to Delay Time (CC#11),
    allowing for manual delay oscillation even while tweaking reverb settings.
  - TRS-MIDI Type A: Directly connects to the DL4 MkII via a standard 3.5mm
    stereo cable (AMEI CA-033/MIDI 1.0 compliant).

# Hardware Recommendations

1. Microcontroller Selection

To utilize all 4 knobs (including Delay Time), you must use a board that exposes
the ADC3 (GP29) pin.

  - Recommended: Waveshare RP2040-Zero or AE-RP2040 (Akizuki). These boards
    provide access to GP29, enabling the full 4-knob functionality.
  - Note on Standard Raspberry Pi Pico: The original Pico board only exposes 3
    ADC pins (GP26-28). If you use a standard Pico, the 4th knob (Delay Time)
    will be unavailable.

2. Encoder and Button

  - Recommended: Use a Rotary Encoder with an integrated push switch. This
    allows you to toggle Reverb Routing (CC#19) by simply pressing the encoder
    knob, keeping the interface compact.
  - Alternative: If your encoder does not have a push mechanism, you can connect
    a separate momentary push button to the designated pin (GP14).

# Bill of Materials (BOM)

| Category      | Item                      | Notes                                               |
| :------------ | :------------------------ | :-------------------------------------------------- |
| **MCU**       | **Waveshare RP2040-Zero** | Recommended (or any Pico-compatible with ADC3/GP29) |
| **Display**   | SSD1306 OLED              | 128x64 resolution, I2C interface                    |
| **Encoder**   | Rotary Encoder            | **Push switch type recommended**                    |
| **Knobs**     | 10kΩ Potentiometer        | 4 units (B-curve recommended)                       |
| **Output**    | 3.5mm Stereo Jack         | Panel mount, TRS                                    |
| **Resistors** | 10Ω and 33Ω               | 1 each, for the MIDI output circuit                 |

# Wiring Diagram

MCU Pin Mapping

| Pin      | Component      | Role               | MIDI CC    |
| :------- | :------------- | :----------------- | :--------- |
| **GP0**  | TRS Tip        | MIDI Signal (TX)   | \-         |
| **3V3**  | TRS Ring       | MIDI Power         | \-         |
| **GND**  | TRS Sleeve     | Common Ground      | \-         |
| **GP2**  | Encoder A      | Rotation Phase A   | \-         |
| **GP3**  | Encoder B      | Rotation Phase B   | \-         |
| **GP14** | **Encoder SW** | **Reverb Routing** | **CC\#19** |
| **GP4**  | OLED SDA       | I2C Data           | \-         |
| **GP5**  | OLED SCL       | I2C Clock          | \-         |
| **GP26** | Pot 1          | Reverb Decay       | CC\#17     |
| **GP27** | Pot 2          | Reverb Tweak       | CC\#18     |
| **GP28** | Pot 3          | Reverb Mix         | CC\#20     |
| **GP29** | Pot 4          | **Delay Time**     | **CC\#11** |

MIDI Output Circuit (Type-A)

[MCU GP0] --- [ 10 Ω Resistor ] ---> TRS Tip<br>
[MCU 3V3] --- [ 33 Ω Resistor ] ---> TRS Ring<br>
[MCU GND] -------------------------> TRS Sleeve<br>

# Software Setup

# Firmware Installation

Choose your preferred method to install the firmware:

Option 1: UF2 Method (Easiest)

1.  Hold the BOOTSEL button on your board while connecting it to your PC via
    USB.
2.  The board will appear as a drive named RPI-RP2.
3.  Drag and drop the provided .uf2 file into that drive.
4.  The board will automatically reboot and start functioning.

Option 2: Arduino IDE (For Customization)

1.  Install the Raspberry Pi Pico/RP2040 core by Earle Philhower.
2.  Install the following libraries via Library Manager:
      - MIDI Library
      - RotaryEncoder
      - Adafruit GFX Library
      - Adafruit SSD1306
3.  Select Board: "Generic RP2040" (required to enable ADC3/GP29).
4.  Open the .ino sketch and click Upload.

# Usage

1.  Connect the controller to the Line 6 DL4 MkII MIDI IN port using a
    standard 3.5mm TRS cable.
2.  Turn Encoder: Change the Reverb Model.
3.  Push Encoder (or Button): Cycle through Reverb/Delay Routing (Pre / Parallel
    / Post).
4.  Turn Potentiometers: Adjust parameters. The OLED will automatically pop up
    the HUD to show precise values and parameter names.

# License

This project is released under the MIT License.
