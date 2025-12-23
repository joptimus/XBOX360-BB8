# XBOX360 BB8

Control a BB-8 style robot (Joe's Drive MK2) using an XBOX 360 wireless controller. This project replaces the original Bluetooth HC05 module with an Arduino-based USB host system for improved reliability and controller support.

## Overview

The system uses two Arduino boards communicating via serial:

- **Remote Arduino (Uno)** - Reads XBOX 360 controller input via USB Host Shield and transmits commands
- **Drive Arduino (Mega)** - Receives commands and controls motors, stabilization, dome movement, and sounds

```
XBOX 360 Controller
        │
        ▼
┌─────────────────┐     Serial      ┌─────────────────┐
│  Arduino Uno    │ ───────────────▶│  Arduino Mega   │
│  + USB Host     │   EasyTransfer  │  (Drive Board)  │
│  (Remote)       │                 │                 │
└─────────────────┘                 └────────┬────────┘
                                             │
                              ┌──────────────┼──────────────┐
                              ▼              ▼              ▼
                          Motors          Dome           Sounds
                         + PIDs        + Sensors
```

## Hardware Requirements

### Remote Unit
| Component | Description |
|-----------|-------------|
| Arduino Uno | Main controller for remote |
| USB Host Shield 2.0 | Enables USB communication |
| XBOX 360 Wireless Receiver | Microsoft PC receiver for XBOX 360 controllers |

### Drive Unit
| Component | Description |
|-----------|-------------|
| Arduino Mega 2560 | Main controller (needs multiple serial ports) |
| Motor Drivers | PWM-controlled H-bridge drivers |
| IMU (MPU6050) | Provides pitch/roll for stabilization |
| Potentiometers | Position feedback for dome tilt, side tilt, dome spin |
| Sound Board | Trigger-based sound effects board |
| Voltage Divider | Battery monitoring circuit |

## Software Requirements

### Libraries
Install these libraries via Arduino IDE Library Manager or GitHub:

| Library | Purpose | Link |
|---------|---------|------|
| USB Host Shield 2.0 | XBOX 360 controller communication | [GitHub](https://github.com/felis/USB_Host_Shield_2.0) |
| EasyTransfer | Serial data transmission | [GitHub](https://github.com/madsci1016/Arduino-EasyTransfer) |
| EEPROMex | Extended EEPROM functions | [GitHub](https://github.com/thijse/Arduino-EEPROMEx) |
| PID_v1 | PID control loops | [Arduino Playground](http://playground.arduino.cc/Code/PIDLibrary) |

## Controller Mapping

| Button | Function |
|--------|----------|
| **START** | Enable/Disable motors (LED indicates state) |
| **BACK** | Cycle drive speed: Slow → Medium → Fast |
| **Left Stick X** | Dome spin |
| **Left Stick Y** | Dome tilt forward/back |
| **Right Stick X** | Side-to-side tilt |
| **Right Stick Y** | Main drive forward/reverse |
| **L2 Trigger** | Spin flywheel left |
| **R2 Trigger** | Spin flywheel right |
| **B Button** | Play random sound |
| **Y Button** | Play music |
| **R3 (Right Stick Click)** | Toggle drive direction |
| **L3 (Left Stick Click)** | Toggle dome servo mode |

### LED Indicators
- **Rotating LEDs** - Motors disabled
- **LED 1** - Slow speed
- **LED 2** - Medium speed
- **LED 3** - Fast speed

## Pin Configuration

The drive board pin assignments are defined at the top of `JamesL_Xbox_Drive.ino`. Key pins:

| Function | Pins |
|----------|------|
| Main Drive PWM | 11, 12 |
| Side-to-Side PWM | 7, 8 |
| Dome Tilt PWM | 5, 6 |
| Dome Spin PWM | 9, 10 |
| Flywheel PWM | 3, 4 |
| Motor Enable | 31 (body), 29 (dome) |
| Side Tilt Pot | A0 |
| Dome Tilt Pot | A1 |
| Dome Spin Pot | A4 |
| Battery Monitor | A3 |
| Sound Pins | 26, 28, 30, 32, 44, 46 |

## Installation

1. **Install Arduino IDE** (1.8.x or 2.x)

2. **Install Libraries** - Use Library Manager or download from links above

3. **Upload Remote Sketch**
   - Open `JamesL_Xbox_Remote/JamesL_Xbox_Remote.ino`
   - Select Board: Arduino Uno
   - Select your COM port
   - Upload

4. **Upload Drive Sketch**
   - Open `JamesL_Xbox_Drive/JamesL_Xbox_Drive.ino`
   - Select Board: Arduino Mega 2560
   - Select your COM port
   - Upload

5. **Connect Hardware**
   - Wire the Remote's TX to Drive's RX1
   - Connect motor drivers, sensors, and sound board per pin configuration
   - Connect IMU to Drive's Serial2

## Configuration

### Reversing Axes
Uncomment these defines in `JamesL_Xbox_Drive.ino` if your axes are reversed:
```cpp
//#define reverseDrive
//#define reverseDomeTilt
//#define reverseS2S
//#define reverseDomeSpin
//#define reverseFlywheel
#define reversePitch    // Usually needed
//#define reverseRoll
```

### Speed Tuning
Adjust these values for your motors:
```cpp
#define easeDome 20           // Dome spin easing (lower = slower)
#define easeDomeTilt .8       // Dome tilt easing
#define flywheelEase 3        // Flywheel acceleration
#define S2SEase 2             // Side-to-side speed
#define MaxDomeTiltAngle 17   // Max dome tilt (max 25)
```

### PID Tuning
Each control loop has configurable PID values:
```cpp
// Side-to-side tilt
double Pk1 = 13, Ik1 = 0, Dk1 = 0.3;

// Side-to-side stability
double Pk2 = 0.5, Ik2 = 0, Dk2 = 0.01;

// Main drive
double Pk3 = 5, Ik3 = 0, Dk3 = 0;

// Dome tilt
double Pk4 = 6, Ik4 = 0, Dk4 = 0.05;

// Dome spin servo
double Kp5 = 4, Ki5 = 0, Kd5 = 0;
```

## Debugging

Enable debug output by uncommenting defines in `JamesL_Xbox_Drive.ino`:
```cpp
//#define printRemote       // Controller values
//#define debugS2S          // Side-to-side PID
//#define debugDrive        // Main drive PID
//#define debugDomeTilt     // Dome tilt PID
//#define debugdomeRotation // Dome spin
//#define printYPR          // IMU pitch/roll
//#define debugFlywheelSpin // Flywheel values
```

Serial output is at 115200 baud.

## Known Issues

1. ~~Turning while driving forward not working~~ (Fixed)
2. Dome spin speed control needs refinement
3. Drive calibration feature incomplete
4. Sound-to-button mapping not implemented

## Safety Features

- **Auto-disable** - Motors disable after 3 seconds of inactivity
- **Controller disconnect** - All channels center and motors disable if controller connection lost
- **Motor enable toggle** - START button must be pressed to enable motors

## Contributors

- **James Lewandowski**
- **Stephane Beaulieu**
- **Joe Latiola** (Original Joe's Drive inspiration)

## License

MIT License - See [LICENSE](LICENSE) file

## Links

- [Joe's Drive Facebook Group](https://www.facebook.com/groups/JoesDrive/)
- [USB Host Shield 2.0 Wiki](https://github.com/felis/USB_Host_Shield_2.0/wiki)
