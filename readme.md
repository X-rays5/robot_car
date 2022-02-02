# Robot car
This program was developed on a ELEGOO smart robot car kit 3.0. While not tested the basic functionality of this program will most likely work, on all the ELEGOO smart robot car kits.

# Technologies
- Languages: c++, typescript
- Libraries: ArduinoJson, IRremote, Svelte, Bootstrap, Font Awesome
- Hardware: ELEGOO smart robot car kit 3.0
- Hosting: Cloudflare pages
- misc: Node.js, BLE, Bluetooth web api

# Usage
arduino setup:
1. Install the https://github.com/bblanchon/ArduinoJson library.
2. Install the https://github.com/Arduino-IRremote/Arduino-IRremote library.
3. Disconnect the bluetooth module from the arduino.
4. Then upload the arduino program car_control/car_control.ino to the arduino.
5. Connect the bluetooth module to the arduino.
6. Turn the car on.
7. Go to https://robot-car.scheenen.dev/ this will always have the latest version of the control_panel running.
8. Connect to the car and have fun!
