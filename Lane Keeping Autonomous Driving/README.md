# Lane Keeping Autonomous Robocar

This project consists of three main code files, along with a camera calibration script and its generated parameter file.

## File Structure
- `arduino.ino` - Arduino UNO firmware
- `esp32s3cam.ino` - ESP32-S3 camera firmware
- `lane_keeping.py` - Main lane keeping algorithm (runs on PC)
- `camera_calibration.ipynb` - Camera calibration notebook
- `perspective_matrix.npy` - Generated camera parameters

## Setup Instructions

### Step 1: Flash Arduino UNO
1. Connect Arduino UNO to your PC.
2. Open `arduino.ino` in Arduino IDE.
3. Select correct board (Arduino UNO) and port.
4. Upload the sketch.

### Step 2: Flash ESP32-S3
1. Connect ESP32-S3 development board to your PC.
2. Open `esp32s3cam.ino` in Arduino IDE.
3. Select correct board (ESP32S3) and port.
4. Upload the sketch.

### Step 3: Run Lane Keeping System
1. Power on the RoboCar.
2. Connect your PC to WiFi network named "ESP32S3CAM".
3. Run `lane_keeping.py` on your PC.

 #### The RoboCar will now perform lane keeping function.

 ### Camera Calibration
 If you need to recalibrate the camera:
1. Run camera_calibration.ipynb notebook.
2. Follow the interactive calibration steps.
3. The notebook will generate new perspective_matrix.npy parameters.




