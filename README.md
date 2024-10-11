# smart-home-display
This is a simple display with 4 LED for use in a Bosch Smart Home system (generation II). It uses the REST API of the Bosch Smart Home controller. The display shows the state of user defined states, which can be created in the BSH app. This display is based on a ESP8266 microcontroller. 

![smart display](https://github.com/tobo-123/smart-home-display/blob/main/pictures/1_small.jpg)

![smart display](https://github.com/tobo-123/smart-home-display/blob/main/pictures/2_small.jpg)


Features:

- 4 LED which can be used independently in the BSH for indicating user defined states, e.g., for indicating open windows, high humidity or as a reminder for the alarm system
- LED can be set off, on and in flashing mode
- power via USB
- LED brightness can be adjusted in the program
- low power consumption (< 0.5 W)

You can see the smart display in use here: https://youtube.com/shorts/-W9cwOug4t0?feature=share


What you need:

- ESP8266 microcontroller (Mini NodeMcu with 16 pins)
- 4x 5mm standard LED (color as you like) + resistors for LED to work on 3.3 V (blue and white can be operated directly; green, yellow and red need a 220 Ohm resistor)
- 3D printed case (.stl files in this repository)
- some wires
- micro USB cable
- soldering iron
- Arduino IDE on your pc
- OpenSLL on your pc
- Arduino program code (in this repository)

How to set it up:

1. Print the case. I use standard white PLA material. Both parts don't need supports. The orientation is with the front / back plate facing downwards. 15 % infill.
2. Insert the 4 LEDs and the ESP8266. Connect the LEDs with resistors to the GPIO pins and ground according to the wiring diagram below.
3. Create a self-signed certificate and key with OpenSSL by using the command: openssl req -x509 -nodes -days 9999 -newkey rsa:2048 -keyout client-key.pem -out client-cert.pem
4. Open the key and certificate file with notepad and copy the text in the corresponding sections of the program code. Also put in your WIFI name, WIFI password, BSH system password and BSH controller IP. The BSH system password needs to be coded in Base64, there are many encoders online to do this for you.
5. Edit the parameter array in the program code: Put in the names of the user defined states of your BSH app, the corresponding LED to switch, brightness level and flashing mode. See comments in the program.
6. Connect the ESP8266 to your PC with the micro USB cable. Press the button on your BSH controller (the middle and right LED of your controller should flash now) and upload and run the program code via the Arduino IDE. You should see each LED of your display flash once. If this does not happen, check whether you have connected the LED correctly and used the correct GPIO pins of the ESP.
7. Check in BSH app if "OSS ESP Display" occurs at mobil devices -> If yes, the display is registered now. If not, check your BSH system password.
8. Now, set the variable register_client = false and upload the program again. The display is ready now! Try to change a user defined state and the corresponding LED should indicate the state.
9. Remove the display from your PC and use it where you want. WIFI reception is needed ;)

Wiring:

![smart display wiring](https://github.com/tobo-123/smart-home-display/blob/main/smart_display_wiring.png)
