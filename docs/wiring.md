# Wiring Guide

This document records the hardware wiring used by the current firmware (`firmware/arduino/code/dexterous_hand_controller.ino`).

## Pin / Interface Mapping

| Module | Connection | Pin / Interface |
|---|---|---|
| LV-TTL Motor 1 | Thumb / pinch finger | `Serial1` (TX1=18, RX1=19) |
| LV-TTL Motor 2 | Index / pinch finger | `Serial2` (TX2=16, RX2=17) |
| PWM Servo 1 | Follower finger 1 | D2 |
| PWM Servo 2 | Follower finger 2 | D6 |
| PWM Servo 3 | Follower finger 3 | D10 |
| Button A (GRIP) | Grasp trigger | D42 |
| Button B (OPEN) | Open / release | D40 |
| Button C | Fine adjustment for thumb (IN) | D25 |
| Button D | Fine adjustment for thumb (OUT) | D27 |
| Button E | Fine adjustment for index (IN) | D29 |
| Button F | Fine adjustment for index (OUT) | D31 |
| Button G (STOP) | Emergency stop | D20 |
| Button H (SYNC) | Synchronized follower fingers | D22 |

## Wiring Notes

- Button inputs use `INPUT_PULLUP` in firmware: released = `HIGH`, pressed = `LOW`.
- Ensure **common ground (GND)** between Arduino, LV-TTL motor power, and servo power.
- Verify power supply current is sufficient for simultaneous servo movement.
- Test with low-speed and emergency STOP available before full-speed operation.

## Firmware Source of Truth

If this document and firmware diverge, firmware definitions are authoritative:

- Button pins: `kBtnOpen`, `kBtnGrip`, `kBtnStop`, `kBtnSync`, `kBtnFineInM1`, `kBtnFineOutM1`, `kBtnFineInM2`, `kBtnFineOutM2`
- PWM pins: `kPwmPins = {2, 6, 10}`
- LV-TTL serial interfaces: comments and motor configuration near the top of the file
