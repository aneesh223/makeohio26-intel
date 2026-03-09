# Subfab Drain Sentry, By Sahil Gandhi, Aneesh Jonnala, and Samanyu Vyakaranam
**MakeOHI/O 2026 — Intel Water Challenge**
**Code written by Aneesh Jonnala**

Real-time semiconductor wastewater classification on a Raspberry Pi Pico 2W. Measures TDS and temperature, applies thermal compensation, and routes water to reuse, filtration, or brine management using SEMI F63 thresholds.

## Routing Logic

| LED | TDS | Temp | Action |
|-----|-----|------|--------|
| 🔵 Solid | < 1 ppm | ≤ 35°C | Direct UPW reuse |
| 🟢 Slow blink | 1–500 ppm | ≤ 35°C | RO/DI filtration |
| 🔴 Fast blink | > 500 ppm | > 35°C | Brine / discard |

## Hardware
- Raspberry Pi Pico 2W
- DFRobot SEN0244 TDS → GP28
- DS18B20 temperature → GP27
- RGB LEDs → GP6, GP12, GP10

## Math
```
coeff  = 1.0 + 0.02 × (T − 25)
V_comp = V_raw / coeff
TDS    = (133.42V³ − 255.86V² + 857.39V) × 0.5
```

## Setup
1. Add Pico 2W board support in Arduino IDE
2. Install `OneWire` and `DallasTemperature`
3. Flash `sketch_final/sketch_final.ino`
4. Serial monitor at 115200 baud
