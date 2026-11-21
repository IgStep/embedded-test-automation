# Wiring: NUCLEO-F446RE, LEDs and Raspberry Pi 4

Both boards use 3.3 V logic, so plain jumper wires are safe. **Never connect
a Pi pin to the Nucleo's 5V pin.**

## LEDs (on the breadboard)
Each pin: Nucleo pin → 330 Ω resistor → LED long leg (anode). LED short leg → GND.

## Nucleo ↔ Pi

| Signal | Nucleo pin | Nucleo header | Pi GPIO (BCM) | Pi header pin | Pi direction |
|---|---|---|---|---|---|
| LED1 | PC0 | CN7-38 | GPIO5 | 29 | input |
| LED2 | PC1 | CN7-36 | GPIO6 | 31 | input |
| LED3 | PC2 | CN7-35 | GPIO13 | 33 | input |
| LED4 | PC3 | CN7-37 | GPIO19 | 35 | input |
| LED5 | PC4 | CN10-34 | GPIO26 | 37 | input |
| LED6 | PC5 | CN10-6 | GPIO16 | 36 | input |
| LED7 | PC6 | CN10-4 | GPIO20 | 38 | input |
| Heartbeat LD2 | PA5 | CN10-11 | GPIO21 | 40 | input |
| Button B1 | PC13 | CN7-23 | GPIO12 | 32 | output low = pressed, input = released |
| Ground | GND | CN7-19 (or CN7-8, CN10-9) | GND | 39 (or 34) | |
| UART + flashing | USB (ST-LINK) | | any USB-A port | | `/dev/ttyACM0` |

```
 NUCLEO-F446RE                              RASPBERRY PI 4
 PC0..PC6 ──┬── 330 Ω ── LED ── GND
            └────────────────────────────── GPIO5,6,13,19,26,16,20  (inputs)
 PA5  (LD2) ─────────────────────────────── GPIO21                  (input)
 PC13 (B1)  ─────────────────────────────── GPIO12   low = "press", input = "release"
 GND  ───────────────────────────────────── GND      connect this first
 USB  ═════════════════════════════════════ USB      serial port + st-flash
```

## Rules
- **Always connect GND.** Without a common ground, the Pi reads noise.
- **The Pi never drives PC13 high.** B1 already has a pull-up on the
  Nucleo. The Pi either pulls the line low (a press) or lets go (input mode).
  So nothing breaks if someone presses the real button during a test.
- **Pi header pins are counted 1..40.** Pin 1 is the one with the square
  solder pad, at the end of the header away from the USB ports. Odd
  numbers are on the inner row, even numbers on the outer row.
- **The Pi powers the Nucleo over USB.** You don't need any other supply.

## Check the wiring before testing
On the Pi, with the firmware running, watch the pins live:
```
watch -n 0.3 pinctrl get 5,6,13,19,26,16,20,21     # Ctrl+C to stop
```
- Exactly one of the first 7 reads `hi`. It walks in LED order
  5 → 6 → 13 → 19 → 26 → 16 → 20 (PC0..PC6), one step per second.
  `pinctrl` sorts the lines by GPIO number, so in the list it seems to jump.
- GPIO21 (LD2) toggles on every step.
- A pin that never reads `hi` has a loose wire. GPIO5 and GPIO6 have a
  pull-up on the Pi, so for them a loose wire reads `hi` all the time.
- Press B1 or send `r`: the `hi` walks the other way.

(`pinctrl` ships with Raspberry Pi OS. On older images use `raspi-gpio get`.)
