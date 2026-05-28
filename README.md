# Multi-Level Security Access System

This project implements a sequential 3-tier authentication system using an ARM7 LPC2148 microcontroller. Designed for high-security environments, it ensures that only authorized personnel can operate specific equipment or gain physical entry.

## 📋 System Features
- **Tier 1 Security (RFID):** Reads 10-byte data frames from an EM-18 RFID reader using asynchronous UART0 interrupts.
- **Tier 2 Security (Keypad & Storage):** Scans a 4x4 matrix keypad for user passcode inputs and validates them against credentials stored in an AT24C256 external EEPROM over I2C lines.
- **Tier 3 Security (Biometrics):** Interfaces with an R305 fingerprint sensor module over serial communication to verify enrolled biometric signatures.
- **Fail-Safe Mechanism:** Includes a real-time software timeout loop. If a user scans their RFID card but fails to complete the passcode entry within ~5 seconds, the session completely resets for security.
- **Admin Override:** Features an external interrupt (EINT1) menu loop that allows system administrators to dynamically edit stored passcodes and manage fingerprint profiles.
- **Output Action:** Successfully verified access triggers an L293D H-bridge driver to control a 12V DC motor, simulating an automated secure door slide sequence.

## 🛠️ Hardware Requirements
- LPC2148 Microcontroller (ARM7)
- EM-18 RFID Reader & Cards
- R305 Fingerprint Module
- 4x4 Matrix Keypad
- AT24C256 EEPROM (I2C)
- 16x2 Character LCD 
- L293D Motor Driver & DC Motor
- Push Button Switch (EINT1 Trigger)

## 💻 Software Stack
- **Language:** Embedded C
- **IDE:** Keil uVision
- **Flashing Tool:** Flash Magic
