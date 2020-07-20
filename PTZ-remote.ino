#include "Arduino.h"

#include <Keypad.h>
#include <SPI.h>
#include <RF24.h>

#include "commands.h"


const byte ROWS = 4; //four rows
const byte COLS = 3; //three columns
char keys[ROWS][COLS] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[ROWS] = {8, 7, 6, 5}; //connect to the row pinouts of the keypad
byte colPins[COLS] = {A0, A1, A2}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

#define joyXPin 7 //!Analog Pin
#define joyYPin 6 //!Analog Pin
// #define joyZPin 3 //!Analog Pin



RF24 radio(10, 9); // CE, CSN

const byte RFaddress[6] = "69489";

enum tallyState {
  program,
  preview,
  none
};

enum buttonState {
  Pressed,
  Hold,
  Released,
  Idle
};

// enum modes {
//   normal,
//   modeWrite
// };


struct dataPackage {
  byte joyX = 127;
  byte joyY = 127;
  byte joyZ = 127;
  // modes mode = normal;
  tallyState tally = none;
  byte speed = 1;
  buttonState button[12] {Idle, Idle, Idle, Idle, Idle, Idle, Idle, Idle, Idle, Idle, Idle, Idle}; // Button formatting = none:0, 1:1, 2:2, 3:3, 4:4, 5:5, 6:6, 7:7, 8:8, 9:9, 0:10, *:11, #:12

  
};

dataPackage data;


unsigned long lastSend;


void setup(){
  Serial.begin(115200);

  //TODO: add joystick detection. If all joystick inputs (or only the z axis) are 0 at bootup there's probably no joystick.

  radio.begin();
  radio.openWritingPipe(RFaddress);
  radio.setPALevel(RF24_PA_MAX);
  radio.stopListening();

  lastSend = millis();
}

void loop(){
  readInputs();

  if (millis() > lastSend + 100){
    lastSend = millis();
    sendJoy();
  }


  if (false){
    lastSend = millis();

    Serial.print("Buttons: ");
    for (int x = 0; x < 12; x++){
      Serial.print(data.button[x]);
      Serial.print(" ");
    }
    Serial.println();
  }

  
  if (data.button[10] == Pressed || data.button[10] == Hold){ //TODO Make it only on hold, will need to add some kind of indicator.

    for (int x = 0; x < 10; x++){
      if (data.button[x] == Pressed){

        const byte send[] = {writePos, 1 + x};


        Serial.print("S: Command: ");
        Serial.print(commandNames[send[0]]);
        Serial.print(" Data: ");
        Serial.println(send[1]);
        

        radio.write(&send, sizeof(send));

        break;
      }
    }

  }
  else {

    for (int x = 0; x < 10; x++){
      if (data.button[x] == Pressed){

        const byte send[] = {callPos, 1 + x, data.speed};


        Serial.print("S: Command: ");
        Serial.print(commandNames[send[0]]);
        Serial.print(" Data: ");
        Serial.print(send[1]);
        Serial.print(" ");
        Serial.println(send[2]);
        

        radio.write(&send, sizeof(send));

        break;
      }
    }


  }

}

void sendJoy() {
  const byte send[] = {joyUpdate, data.joyX, data.joyY, 127, data.speed};

  Serial.print("S: Command: ");
  Serial.print(commandNames[send[0]]);
  Serial.print(" Data: ");
  Serial.print(send[1]);
  Serial.print(" ");
  Serial.print(send[2]);
  Serial.print(" ");
  Serial.print(send[3]);
  Serial.print(" ");
  Serial.println(send[4]);
  
  radio.write(&send, sizeof(send));
}


void readInputs() {
  #ifdef joyXPin
    if (analogRead(joyXPin) < 485 || analogRead(joyXPin) > 539){
      data.joyX = map(analogRead(joyXPin), 0, 1023, 0 , 255);
    }
    else{
      data.joyX = 127;
    }
    
  #endif

  #ifdef joyYPin
    if (analogRead(joyYPin) < 485 || analogRead(joyYPin) > 539){
      data.joyY = map(analogRead(joyYPin), 0, 1023, 0 , 255);
    }
    else{
      data.joyY = 127;
    }

  #endif

  #ifdef joyZPin
    if (analogRead(joyZPin) < 485 || analogRead(joyZPin) > 539){
      data.joyZ = map(analogRead(joyZPin), 0, 1023, 0 , 255);
    }
    else{
      data.joyZ = 127;
    }

  #endif




  if (keypad.getKeys())
  {

    for (int i=0; i<LIST_MAX; i++)   // Scan the whole key list.
    {
      if ( keypad.key[i].stateChanged )   // Only find keys that have changed state.
      {
        int button;
        switch (keypad.key[i].kchar){

          case '1': 
          button = 0;
          break;

          case '2': 
          button = 1;
          break;

          case '3': 
          button = 2;
          break;

          case '4': 
          button = 3;
          break;

          case '5': 
          button = 4;
          break;

          case '6': 
          button = 5;
          break;

          case '7': 
          button = 6;
          break;

          case '8': 
          button = 7;
          break;

          case '9': 
          button = 8;
          break;

          case '0': 
          button = 9;
          break;

          case '*': 
          button = 10;
          break;

          case '#': 
          button = 11;
          break;
        }

        switch (keypad.key[i].kstate) {  // Report active key state : IDLE, PRESSED, HOLD, or RELEASED
          case PRESSED:
          data.button[button] = Pressed;
          break;

          case HOLD:
          data.button[button] = Hold;
          break;

          case RELEASED:
          data.button[button] = Released;
          break;

          case IDLE:
          data.button[button] = Idle;
        }

      }
    }
  }


}
