# smart-home-display
Simple LED-based display for use in a Bosch Smart Home system

This is a simple display with 4 LED for use in a Bosch Smart Home system (Generation II). It communicates via ZigBee. The display can be used in automations for indicating actions and states of your smart home. Since the Bosch Smart Home System (BSH) is a closed system, we need a device which can be registered to the BSH. I use a Ledvance Smart+ LED flex controller. This controller is added to the BSH as a lamp. By sending defined colors to this lamp, it can switch LED of this display on or off. This is controlled by a ESP8266 microcontroller. 

![smart display](https://github.com/tobo-123/smart-home-display/blob/main/pictures/1_small.jpg)

![smart display](https://github.com/tobo-123/smart-home-display/blob/main/pictures/2_small.jpg)

What you need:

- Ledvance Smart+ Controller, part of Ledvance smart+ flex set 4058075208339
- ESP8266 microcontroller (Mini NodeMcu with 16 pins)
- 4x 5mm standard LED (color as you like) + resistors for LED to work on 3.3 V (blue and white can be operated directly; green, yellow and red need a 220 Ohm resistor)
- 2x 10k pull-up resistors
- 3D printed case (.stl files in this repository)
- some wires
- micro USB cable
- soldering iron
- Arduino IDE on your pc
- Arduino program code (in this repository)

How to set it up:

1. Connect the controller with the power supply of the Ledvance set. Add the Ledvance controller to your BSH by scanning the QR code.
2. Disconnect the controller from the power supply and open the plastic housing of the controller. This is only clipped on. Then remove the circuit board and cut off the cables.
3. Print the new case. I use standard white PLA material. Both parts don't need supports. The orientation is with the front / back plate facing downwards. 15% infill.
4. Insert the 4 LEDs.
5. Connect the LED according to the circuit diagram. Insert the ESP and the Ledvance circuit board and connect everything according to the circuit diagram. See also the real life picture at the end of this readme.
6. Connect the ESP to your PC with the micro USB cable and upload the program code via the Ardunino IDE. After restarting and waiting 10s, you should see each LED flash once. If this does not happen, check whether you have connected the LED correctly and used the correct GPIO pins of the ESP. You can also change the GPIO numbers in the program code. See the comments in the code.
7. Check if you still find your Ledvance lamp in BSH app. If not available, check your wiring.
8. You need for each LED two defined colors to switch them on or off. Therefore, define 8 colors in your BSH smart home app for the Ledvance lamp. Only use colors with full brightness, no dimming!
9. Open the serial monitor in Arduino IDE (rate: 115200). Now, set the lamp to a defined color with the BSH app. On the serial monitor, you should see the red and blue value of this color as text massage. Note them. Then, switch the lamp off via BSH app and wait for at least 2 seconds. Repeat that for all 8 colors and note the red and blue values.
10. At the beginning of the Arduino code, you will find an array. This array defines the colors which activate or deactivate a certain LED state. Use the red and blue color values you have noted and add them with a safety margin of +/-10 in the array. For example: If your red value is 950 and your blue value is 400 for a defined color, add 940 / 960 as lower/upper red value and 390/410 as lower/upper blue value in the array. See also the commends in the code.
11. Upload the changed program. Now, you can use the colors in BSH app. By sending a defined color, you trigger the corresponding state in the array and activate or deactivate a LED. Set the lamp to a defined color for at least 2 seconds and switch off the lamp afterwards. After 2 more seconds, the display is ready to recieve the next "color command". (We need to wait at least 2 seconds because the Ledvance lamp fades from zero to the final color. So we need to wait until the color is reached, then we can read the red and blue value. Same happens when we deactivate the lamp).

Wiring:

![smart display wiring](https://github.com/tobo-123/smart-home-display/blob/main/smart_display_wiring.png)

How it looks in real life:

![smart display wiring](https://github.com/tobo-123/smart-home-display/blob/main/pictures/3.jpg)
