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
byte colPins[COLS] = {4, 3, 2}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, ROWS, COLS );

#define joyXPin 0 //!Analog Pin
#define joyYPin 1 //!Analog Pin
// #define joyZPin 2 //!Analog Pin



RF24 radio(10, 9); // CE, CSN

const byte RFaddress[6] = "69489";

enum tallyState {
  program,
  preview,
  none
};

enum modes {
  normal,
  modeWrite
};


struct Data_Package {
  byte joyX = 127;
  byte joyY = 127;
  byte joyZ = 127;
  modes mode = normal;
  tallyState tally = none;
  byte speed = 1;
  byte button = 0; // Button formatting = none:0, 1:1, 2:2, 3:3, 4:4, 5:5, 6:6, 7:7, 8:8, 9:9, 0:10, *:11, #:12

  
};

Data_Package data;


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
  
  if (data.button == 11){
    if (data.mode == normal){
      data.mode = modeWrite;
    }
    else {
      data.mode = normal;
    }
  }
  else if (data.button != 0 && data.button != 11 && data.button != 12 &&  data.mode == normal){

    const byte send[] = {callPos, data.button, data.speed};

    Serial.print("S: Command: ");
    Serial.print(commandNames[send[0]]);
    Serial.print(" Data: ");
    Serial.print(send[1]);
    Serial.print(" ");
    Serial.println(send[2]);
    

    radio.write(&send, sizeof(send));
  }

  else if (data.button != 0 && data.button != 11 && data.button != 12 && data.mode == modeWrite){

    const byte send[] = {writePos, data.button};

    Serial.print("S: Command: ");
    Serial.print(commandNames[send[0]]);
    Serial.print(" Data: ");
    Serial.println(send[1]);

    radio.write(&send, sizeof(send));
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


  char key = keypad.getKey();

  if (key != NO_KEY){
    if (key == '1'){
      data.button = 1;
    }
    else if (key == '2'){
      data.button = 2;
    }
    else if (key == '3'){
      data.button = 3;
    }
    else if (key == '4'){
      data.button = 4;
    }
    else if (key == '5'){
      data.button = 5;
    }
    else if (key == '6'){
      data.button = 6;
    }
    else if (key == '7'){
      data.button = 7;
    }
    else if (key == '8'){
      data.button = 8;
    }
    else if (key == '9'){
      data.button = 9;
    }
    else if (key == '0'){
      data.button = 10;
    }
    else if (key == '*'){
      data.button = 11;
    }
    else if (key == '#'){
      data.button = 12;
    }
    
  }
  else {
    data.button = 0;
  }


}

