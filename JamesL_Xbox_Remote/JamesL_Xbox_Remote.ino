// =======================================================================================
//    Joe's Drive Remote: XBOX 360 Controller
// =======================================================================================
//
//                          Last Revised Date: 02/26/2018
//                      Written By: Stephane Beaulieu and James Lewandowski
//
// =======================================================================================
//
//         This program is free software: you can redistribute it and/or modify it .
//         This program is distributed in the hope that it will be useful,
//         but WITHOUT ANY WARRANTY; without even the implied warranty of
//         MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
//
// =======================================================================================
//   Note: You will need a Arduino Uno to run this sketch
//
//   This is written to be a UNIVERSAL Sketch - supporting multiple controller options
//   for the Joe Latiola BB8 build version MK 2.. in replacement of his custom controller
//   see: www.facebook.com/joesdrive
//
//   Libraries Needed
//      - EasyTransfer : https://github.com/madsci1016/Arduino-EasyTransfer
//      - USBHost2.0   : https://github.com/felis/USB_Host_Shield_2.0
//
//   Latest Changes
//
//
// =======================================================================================

//----------------------
// Begin Define Buttons
//----------------------
// XBOX Start = motorEnable   *** Enable/Disable Drive Motor
// XBOX Back =  but6Speed     *** Change Drive Speed, Slow/Med/Fast. Show 1 LED for slow, 2 for med, 3 for fast
// XBOX A =
// XBOX B =     but2          *** Make Sounds
// XBOX X = 
// XBOX Y =     but3          *** Plays music
// XBOX UP =                  Auto Function (Later)
// XBOX DOWN =                Auto Function (Later)
// XBOX LEFT =                Auto Function (Later) 
// XBOX RIGHT =               Auto Function (Later)
// XBOX L1 =
// XBOX R1 =
// XBOX L2 =    ch5           *** Flywheel Left
// XBOX R2 =    ch5           *** Flywheel Right              
// XBOX L3 =    but1          *** Press in on left joystick (L3)
// XBOX R3 =    fwdState      *** Press in on right joystick (R3)

//-----------------------
// Libraries to include
//-----------------------
#include <XBOXRECV.h>
#include <EasyTransfer.h>

//--------------------------------------------------------
// Create the USB Class and the XBOX 360 Controller Class
//--------------------------------------------------------
USB Usb;
XBOXRECV Xbox(&Usb);

//------------------------------------------------
// Create Objects for EasyTransfer on Serial Port
//------------------------------------------------
EasyTransfer SendRemote;

//----------------------------------------
// Structure of Data to send to the body
//----------------------------------------
struct SEND_DATA_STRUCTURE{
  int ch1 = 256;        //main drive                   *****
  int ch2 = 256;        //tilt / steer                 *****
  int ch3 = 256;        //head tilt                    *****
  int ch4 = 256;        //head spin                    *****
  int ch5 = 256;        //spin Flywheel                *****
  int but1 = 0;         //Select on left joystick      *****
  int but2 = 0;         //left 1 Play Sounds
  int but3 = 0;         //left 2 Play Music
  int but4 = 0;         //left 3
  int fwdState = 0;     //Select on right joystick     *****
  int but6Speed = 1;    //Back button                  *****
  int but7 = 0;         //right 2
  int but8 = 0;         //right 3
  int motorEnable = 0;  //toggle on top                *****

};

//---------------------------------------------
// Give a Name to the Group Easy Transfer Data
//---------------------------------------------
SEND_DATA_STRUCTURE sendToBody;

//-----------------------
// Variables Definition
//-----------------------
bool debug = false;
unsigned long lastSendMillis;
boolean firstLoadOnConnect = false;
int JoyLeftX  = 0;
int JoyLeftY  = 0;
int JoyRightX = 0;
int JoyRightY = 0;
int buttonR = 0;

void setup() {
  //---------------------------
  // Serial Port Speed Setting
  //---------------------------
  Serial.begin(115200);

  //---------------------
  // Startup of USB Port
  //---------------------
  #if !defined(__MIPSEL__)
    while (!Serial); // Wait for serial port to connect 
  #endif
  if (Usb.Init() == -1) {
    Serial.print(F("\r\Xbox Controller did not start"));
    while (1); //halt
  }

  //-----------------------------------------------------------
  // Setup the Easy Transfer protocol for Serial Communication
  //-----------------------------------------------------------
  SendRemote.begin(details(sendToBody), &Serial);
  
  Serial.println(F("--------------------------------------"));
  Serial.println(F("    Version 1.0 of XBOX 360 to BB-8   "));
  Serial.println(F("--------------------------------------"));
  Serial.println(F("        By  Joe Latiola"));
  Serial.println(F("            James Lewandowski"));
  Serial.println(F("            Stephane Beaulieu"));
  Serial.println(F("--------------------------------------"));
  Serial.println(F("            February 2018"));
  Serial.println(F("--------------------------------------"));
  Serial.println(F(""));
  Serial.println(F("Xbox Wireless Receiver Library Started"));
}

void loop() {
  //---------------
  // Read USB port
  //---------------
  Usb.Task();
  
  //--------------------------------------------------
  // Check for error condition on the XBOX 360 Remote
  // set all Joystick to 255 if we lose connection 
  //--------------------------------------------------
  if (!Xbox.XboxReceiverConnected || !Xbox.Xbox360Connected[0]) {
    sendToBody.ch1 = 256;       //main drive
    sendToBody.ch2 = 256;       //tilt / steer
    sendToBody.ch3 = 256;       //head tilt
    sendToBody.ch4 = 256;       //head spin
    sendToBody.ch5 = 256;       //spin Flywheel
    sendToBody.motorEnable = 0; //toggle on top
    
    return; // Get out of the loop
  }

  if (!firstLoadOnConnect) {
    firstLoadOnConnect = true;
    Xbox.setLedMode(ROTATING, 0);
  }
  
  //-------------------
  // Read Sticks Value
  //-------------------
  JoyLeftX = Xbox.getAnalogHat(LeftHatX, 0);
  JoyLeftY = Xbox.getAnalogHat(LeftHatY, 0);
  JoyRightX = Xbox.getAnalogHat(RightHatX, 0);
  JoyRightY = Xbox.getAnalogHat(RightHatY, 0);

  //----------------
  // Compute Sticks
  //----------------
  // Check if Left Stick X is Out of the Dead Zone
  if (JoyLeftX > 7500 || JoyLeftX < -7500) {
    sendToBody.ch4 = constrain(map(JoyLeftX, -32000, 32000, 0, 512), 0, 512);
  } else {
    sendToBody.ch4 = 256;  // Stick is Centered
  }
  
  // Check if Left Stick Y is Out of the Dead Zone
  if (JoyLeftY > 7500 || JoyLeftY < -7500) {
    sendToBody.ch3 = constrain(map(JoyLeftY, -32000, 32000, 0, 512), 0, 512);
  } else {
    sendToBody.ch3 = 256;  // Stick is Centered
  }

  // Check if Right Stick X is Out of the Dead Zone
  if (JoyRightX > 7500 || JoyRightX < -7500) {
    sendToBody.ch2 = constrain(map(JoyRightX, -32000, 32000, 0, 512), 0, 512);
  } else {
    sendToBody.ch2 = 256;  // Stick is Centered
  }
  
  // Check if Right Stick Y is Out of the Dead Zone
  if (buttonR == 0 && JoyRightY > 7500 || JoyRightY < -7500){
    sendToBody.ch1 = constrain(map(JoyRightY, -32000, 32000, 0, 512), 0, 512);
  } else if (buttonR == 1 && JoyRightY > 7500 || JoyRightY < -7500) {
    sendToBody.ch1 = constrain(map(JoyRightY, -32000, 32000, 512, 0), 512, 0);
  } else {
    sendToBody.ch1 = 256;  // Stick is Centered
  }

  //------------------------------
  // Read and Compute All Buttons
  //------------------------------

  // Start Button Enable/Disable Drive and Set LED acording to Speed Setting
  if (Xbox.getButtonClick(START, 0)) {
    if(sendToBody.motorEnable == 0){
      sendToBody.motorEnable = 1;
      if(sendToBody.but6Speed == 1){
        Xbox.setLedOn(LED1, 0);  
      } else if(sendToBody.but6Speed == 2) {
        Xbox.setLedOn(LED2, 0);
      } else {
        Xbox.setLedOn(LED3, 0);
      }
    } else {
      sendToBody.motorEnable = 0;
      Xbox.setLedMode(ROTATING, 0);
    }
  }

  // Back Button Change Speed Setting from 1 to 3
  if (Xbox.getButtonClick(BACK, 0)) {
    if(sendToBody.but6Speed == 1){
      sendToBody.but6Speed = 2;
      Xbox.setLedOn(LED2, 0);
    } else if (sendToBody.but6Speed == 2){
      sendToBody.but6Speed = 3;
      Xbox.setLedOn(LED3, 0);  
    } else {
      sendToBody.but6Speed = 1;
      Xbox.setLedOn(LED1, 0);  
    }
  }
  
  // Up
  if (Xbox.getButtonClick(UP, 0)) {}

  // Down    
  if (Xbox.getButtonClick(DOWN, 0)) {}

  // Left
  if (Xbox.getButtonClick(LEFT, 0)) {}

  // Right
  if (Xbox.getButtonClick(RIGHT, 0)) {}

  // A
  if (Xbox.getButtonClick(A, 0)) {}

  // B to Play Sounds
  if (Xbox.getButtonPress(B, 0)){
    sendToBody.but2 = 1;
  }

  // X
  if (Xbox.getButtonClick(X, 0)){  }

  // Y to Play Music
  if (Xbox.getButtonPress(Y, 0)){
    sendToBody.but3 = 1;
  }

  // L2 Trigger Button To Spin Left   
  if (Xbox.getButtonPress(L2, 0)) {
    sendToBody.ch5 = 0;
  }

  // R2 Trigger Button To Spin Right
  if (Xbox.getButtonPress(R2, 0)) {
    sendToBody.ch5 = 512;
  }

  // L1 Trigger Button
  if (Xbox.getButtonClick(L1, 0)) {  }

  // R1 Trigger Button
  if (Xbox.getButtonClick(R1, 0)) {  }

  // L3 Hat Left Stick
  if (Xbox.getButtonClick(L3, 0)) {  }

//  // R3 Hat Right Stick to Change Direction
//  if (Xbox.getButtonClick(R3, 0)){
//    if (sendToBody.fwdState == 0){
//      sendToBody.fwdState = 1;  
//    } else {
//      sendToBody.fwdState = 0;
//    }
//  }

    // R3 Hat Right Stick to Change Direction
  if (Xbox.getButtonClick(R3, 0)){
    buttonR = 1;
    } else {
      buttonR = 0;
    }
  }

  //-------------------------------------------
  // James' Playground
  //-------------------------------------------  
//void readInputs(){
//      state = digitalRead(btStatePin);  // check to see when BT is paired
//    
//      ch1a = analogRead(rVertical); 
//      ch2a = analogRead(rHorizontal);  
//      ch3a = analogRead(lVertical);    
//      ch4a = analogRead(lHorizontal);  
//      ch5a = analogRead(Flywheel);
//      sendToBody.fwdState = Fwd;  
//      but5 = digitalRead(rSelect);
//      
//    
//      if (ch1a == ch1Center){
//        ch1b = 256;
//      }else if (ch1a > ch1Center){
//        ch1b = map(ch1a, ch1Center, 1023, 255, 0);
//      }else if (ch1a < ch1Center){
//        ch1b = map(ch1a, 0, ch1Center, 512, 257);
//      }
//      
//      if (ch2a == ch2Center){
//        ch2b = 256;
//      }else if (ch2a > ch2Center){
//        ch2b = map(ch2a, ch2Center, 1023, 255, 0);
//      }else if (ch2a < ch2Center){
//        ch2b = map(ch2a, 0, ch2Center, 512, 257);
//      }
//    
//      if (ch3a == ch3Center){
//        ch3b = 256;
//      }else if (ch3a > ch3Center){
//        ch3b = map(ch3a, ch3Center, 1023, 255, 0);
//      }else if (ch3a < ch3Center){
//        ch3b = map(ch3a, 0, ch3Center, 512, 257);
//      }
//
//      
//      if(Fwd == 1){
//        sendToBody.ch1 = map(ch1b, 0, 512, 512, 0);
//        sendToBody.ch2 = map(ch2b, 0, 512, 512, 0);
//        sendToBody.ch3 = map(ch3b, 0, 512, 512, 0);
//      }else{
//        sendToBody.ch1 = ch1b;
//        sendToBody.ch2 = ch2b;
//        sendToBody.ch3 = ch3b;
//      }
//
//    
//      if (ch4a == ch4Center){
//        sendToBody.ch4 = 256;
//      }else if (ch4a > ch4Center){
//        sendToBody.ch4 = map(ch4a, ch4Center, 1023, 255, 0);
//      }else if (ch4a < ch4Center){
//        sendToBody.ch4 = map(ch4a, 0, ch4Center, 512, 257);
//      }
//
//      if (ch5a == ch5Center){
//        sendToBody.ch5 = 256;
//      }else if (ch5a > ch5Center){
//        sendToBody.ch5 = constrain(map(ch5a, ch5Center, 780, 255, 0),0,512);
//      }else if (ch5a < ch5Center){
//        sendToBody.ch5 = constrain(map(ch5a, 140, ch5Center, 512, 257),0,512);
//      }
//      
//    
//      if (sendToBody.but8 == 0 && sendToBody.but7 == 0){
//         timeJoystickCalibration();
//      }else if (sendToBody.but8 == 1 || sendToBody.but7 == 1 || sendToBody.motorEnable == 0){
//          joystickCalibState = 0;
//      }
//
//      if(but6 == 0 && but6State == 0){
//        setDriveSpeed();
//      }else if(but6 == 1 && but6State ==1){
//        but6State = 0; 
//      }

  //-------------------------------------------
  // Send data to the Body via the Serial Port
  //-------------------------------------------  
  if (millis() - lastSendMillis >= 20){
    lastSendMillis = millis();
    if(!debug){
      SendRemote.sendData();
      sendToBody.but2 = 0;
      sendToBody.but3 = 0;
      sendToBody.ch5 = 256;
    }
    
    if(debug){
      Serial.print(sendToBody.ch1);
      Serial.print("\t");
      Serial.print(sendToBody.ch2);
      Serial.print("\t");
      Serial.print(sendToBody.ch3);
      Serial.print("\t");
      Serial.print(sendToBody.ch4);
      Serial.print("\t");
      Serial.print(sendToBody.ch5);
      Serial.print("\t");
      Serial.print(sendToBody.but1);
      Serial.print("\t");
      Serial.print(sendToBody.but2);
      Serial.print("\t");
      Serial.print(sendToBody.but3);
      Serial.print("\t");
      Serial.print(sendToBody.but4);
      Serial.print("\t");
      Serial.print(sendToBody.fwdState);
      Serial.print("\t");
      Serial.print(sendToBody.but6Speed);
      Serial.print("\t");
      Serial.print(sendToBody.but7);
      Serial.print("\t");
      Serial.print(sendToBody.but8);
      Serial.print("\t");
      Serial.print(sendToBody.motorEnable);
      Serial.println("\t");
      sendToBody.but2 = 0;
      sendToBody.but3 = 0;
      sendToBody.ch5 = 256;
    }
  }
}


