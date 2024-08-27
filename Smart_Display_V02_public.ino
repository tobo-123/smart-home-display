/*##############################################################################
#   Smart Home Display                                                         #
#   Last change:   19.08.2024                                                  #
#   Version:       0.2 public                                                  #
#   Author:        tobo_123                                                    #
#   Github:        tobo_123                                                    # 
################################################################################
  
Copyright (c) 2024 tobo_123

MIT License

Permission is hereby granted, free of charge, to any person obtaining a copy
of this software and associated documentation files (the "Software"), to deal
in the Software without restriction, including without limitation the rights
to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
copies of the Software, and to permit persons to whom the Software is furnished
to do so, subject to the following conditions:
 
The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE
SOFTWARE.

#################################################################################

Change log:
- First version
- Flashing LED function

##################################################################################*/


#include <Wire.h>
#include <ESP8266WiFi.h>


/*##################################################################################
#####   !!!!!     Adjust / check the following entries   !!!!!    ##################
##################################################################################*/


// GPIO pin numbers, please check or change

const int Pin_Red = 14;   // pin for connecting red PWM signal from Ledvance board
const int Pin_Blue = 16;  // pin for connecting blue PWM signal from Ledvance board
const int Pin_Led1 = 5;   // #1 LED, e.g., red
const int Pin_Led2 = 4;   // #2 LED, e.g., blue
const int Pin_Led3 = 0;   // #3 LED, e.g., yellow
const int Pin_Led4 = 12;  // #4 LED, e.g., white


// number of entries in param array. Usually two per LED (on and off) = 8 entries. Change, if you would like to use more entries

const int number = 8;   


// param is the array for the ranges of red and blue signal to activate a led state. It has this scheme: lower value red / upper value red / lower value blue / upper value blue / led_pin number / PWM brightness from 0 to 255 / flashing mode 0 = off / 1 = on)
// the entries below are just examples and must be adjusted to your colors
// the first example entry means: If a color has a red value between 970 and 999 and the blue value is between 35 and 60, then set LED #1 to brighntess 200. The LED should not flash.

const int param[number][7] = {{970, 999, 35, 60, Pin_Led1, 200, 0},     //on state for LED #1
                              {970, 999, 120, 150, Pin_Led1, 0, 0},     //off state for LED #1
                              {80, 110, 970, 999, Pin_Led2, 25, 0},     
                              {410, 440, 970, 999, Pin_Led2, 0, 0},  
                              {970, 999, 5, 34, Pin_Led3, 255, 1},      
                              {150, 180, 90, 120, Pin_Led3, 0, 0},   
                              {840, 880, 970, 999, Pin_Led4, 12, 0},   
                              {970, 999, 220, 250, Pin_Led4, 0, 0}};    
     


/*##################################################################################
#######   After here, nothing needs to be changed   ################################
##################################################################################*/ 

bool state[number] = {false};   // this array safes the states of led

int red = 0;
int blue = 0;
float avg_red = 0;
float avg_blue = 0;

bool read = false;
bool low = false;
unsigned long current_time = 0;
unsigned long prev1_time = 0;
unsigned long prev2_time = 0;

// ##################################################################################

void setup() {

  WiFi.mode(WIFI_OFF);
  WiFi.forceSleepBegin();         // deactivate WIFI to reduce energy consumption

  Serial.begin(115200);

// pin assignment  

  pinMode(Pin_Red, INPUT);
  pinMode(Pin_Blue, INPUT);
  pinMode(Pin_Led1, OUTPUT);
  pinMode(Pin_Led2, OUTPUT);
  pinMode(Pin_Led3, OUTPUT);
  pinMode(Pin_Led4, OUTPUT);
  pinMode(LED_BUILTIN, OUTPUT);

  digitalWrite(LED_BUILTIN, HIGH);  // deactivates the internal LED

// give the zigbee module time to connect

  delay(10000);

// short flash of all LED to show ESP is running and all LED are working

  digitalWrite(Pin_Led1, HIGH); 
  digitalWrite(Pin_Led2, LOW);   
  digitalWrite(Pin_Led3, LOW);  
  digitalWrite(Pin_Led4, LOW);   
  delay (500);
  digitalWrite(Pin_Led1, LOW);  
  digitalWrite(Pin_Led2, HIGH); 
  delay (500);
  digitalWrite(Pin_Led2, LOW);
  digitalWrite(Pin_Led3, HIGH);
  delay (500);
  digitalWrite(Pin_Led3, LOW); 
  digitalWrite(Pin_Led4, HIGH);
  delay (500);
  digitalWrite(Pin_Led4, LOW);
 
  prev1_time = millis();
  prev2_time = millis();

  Serial.println("Smart display is ready");

}

//##################################################################################

void loop() {

  current_time = millis();

  // if red channel sees something and read = false and low = true -> start read in mode

  if ((pulseIn(Pin_Red, LOW, 10000) != 0) && (!read) && (low)) {
    read = true;
    low = false;
    prev1_time = current_time;
    Serial.println("Something detected!");
  }

   // if it is time to read, read

  if ((read)  && ((current_time - prev1_time) > 1500)) {    // when 1500 ms are over -> read channels

    red = 0;
    blue = 0;
    read = false;

    for (int i = 0; i < 5; i++) {                           // read in 5x times, then averaging
      red = red + pulseIn(Pin_Red, LOW, 10000);
      blue = blue + pulseIn(Pin_Blue, LOW, 10000);
      delay (10);
    }

    avg_red = red / 5.0;
    avg_blue = blue / 5.0;
    avg_blue = int(avg_blue);
    avg_red = int(avg_red);

    Serial.println("Red: " + String(avg_red) + " Blue: " + String(avg_blue));

    // after reading: checking if read in value belongs to parameter set
    
    for (int i = 0; i < number; i++) {
      if ((avg_red > param[i][0]) && (avg_red < param[i][1]) && (avg_blue > param[i][2]) && (avg_blue < param[i][3])) {
        state[i] = true;                                 // set state on in state array
        analogWrite(param[i][4], param[i][5]);           // set according LED pin
 
        //set all other states of the same LED pin to false, except the one currently found

        Serial.println("Param number: " + String(i));

        for (int n = 0; n < number; n++) {
          if ((param[n][4] == param[i][4]) && (n != i)) {
            state[n] = false;
          }
        }
      }
    }
  }

  // Let LED flash, where flashing is enabled and state is on

  if (((current_time - prev2_time) > 500) && ((current_time - prev2_time) < 699)) {

    for (int i = 0; i < number; i++) {
      if ((param[i][6] == 1) && (state[i])) {
        analogWrite(param[i][4], param[i][5]); // turns LED on
      }
    }
  }

  if ((current_time - prev2_time) > 1000) {

    for (int i = 0; i < number; i++) {
      if ((param[i][6] == 1) && (state[i])) {
        analogWrite(param[i][4], 0); // turns LED off
      }
    }
    prev2_time = current_time;
  }


  // wait until red channel is on zero level, make ready for next read mode

  if ((pulseIn(Pin_Red, LOW, 10000) == 0) && (!low)) {
     low = true;
     Serial.println("Ready for reading next color");
  }

  delay (100);

}