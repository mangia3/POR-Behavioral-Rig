# POR-Behavioral-Rig
**Purpose**: The main purpose of the POR Behavioral Rig is to automate the experimentation process for studying physiological responses (such as palp opening response or POR) in locusts due to various odors. This code controls a compact, centralized physical rig to carry out trials. Ultimately this works toward making research processes more efficient and aiding in quicker, more accurate experimental trial conduction.

## Overview
This system automates palp opening response (POR) experiments in locusts by controlling:
- Dual-camera video recording
- Precise odor delivery (8 channels)
- Motorized positioning of test subjects
- Synchronized experiment timing

## Key Components
1. **Hardware**:
   - Raspberry Pi 5 (main controller)
   - Arduino Teensy 4.1 (motor/odor control)
   - TMC2209 stepper driver
   - 2x cameras (left/right)
   - 2x mechanical limit switches
   - Olfactometer (relay boards and air manifolds)
   - 24V PSU

2. **Software**:
   - `rig.py` (Main control script)
   - `motor_controller.py` (Motor interface)
   - `Rig_program.ino` (Arduino firmware)
   - `rig_motor_functions.h` (Motor library)

[![Pinout Diagram](https://drive.google.com/file/d/1OPlse6kRpduqb8w6OBmW3Q2100KdvlVt/view?usp=sharing)](https://drive.google.com/file/d/1OPlse6kRpduqb8w6OBmW3Q2100KdvlVt/view?usp=sharing)
![Pinout Diagram](https://drive.google.com/uc?export=view&id=1OPlse6kRpduqb8w6OBmW3Q2100KdvlVt)
*Fig 1. System pinout and connections diagram*

## Installation
1. Flash Arduino Teensy with `Rig_program.ino` and `rig_motor_functions.h` in the same sketch
2. Install Python dependencies:
`pip install pyserial`
3. Install `rpicam-vid` for camera control

## Usage
1. Connect all hardware components (see [Pinout Diagram](https://drive.google.com/file/d/1OPlse6kRpduqb8w6OBmW3Q2100KdvlVt/view))
2. Run main script:
`python3 rig.py`
3. Videos save in `MMDDYYYY/odor_X/locust_Y/` structure

## Configuration
Modify `rig.py` variables:
```python
TRIALS = 3         # Trials per odor
ODORS = 8          # Number of odors
NUM_LOCUSTS = 6    # Test subjects
PRE_ODOR = 2       # Seconds before odor
ODOR_ON = 4        # Odor duration
POST_ODOR = 2      # Seconds after odor
```

Safety Features
- Hardware limit switches
- Software movement constraints
- Emergency stop command
- Resonance-reducing movement algorithms

Documentation
Full documentation available in project files and:
[Google Drive Folder]([url](https://drive.google.com/drive/folders/1eI-NW90LWxW4njzedlkQXaeBQSi5hJCD?usp=sharing))

Troubleshooting
- Motor not moving:
  - Check limit switches
  - Verify serial connection
  - Confirm power to stepper driver
- Odors not activating:
  - Check manifold wiring
  - Verify GND/VM connection
  - Verify Arduino pin connections
- Camera issues:
  - Ensure `rpicam-vid` is installed
  - Check camera indexes in `rig.py`
  - Check camera commands in `rig.py`

