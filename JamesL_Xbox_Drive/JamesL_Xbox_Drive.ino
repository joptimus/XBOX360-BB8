// ===================================================================================================================================================================================================== 
//                ***         Joe's Drive  - XBOX 360/ONE Controlled Main Drive Mechanism - Updated 3/12/18                                              ***
//                ***                                                                                                                                    ***
//                ***         You are free to use, and share this code as long as it is not sold. There is no warranty, guarantee, or other tomfoolery.  ***
//                ***         This entire project was masterminded by an average Joe, your mileage may vary.                                             ***
//                ***                                                                                                                                    ***
// ===================================================================================================================================================================================================== 
//                            Modified by James Lewandowski and Stephane Beaulieu
//                            Inspired by Joe Latiola: https://www.facebook.com/groups/JoesDrive/
//                            You will need libraries: EepromEX: https://github.com/thijse/Arduino-EEPROMEx
//                                                     PIDLibrary: http://playground.arduino.cc/Code/PIDLibrary  
//                                                     EasyTransfer: https://github.com/madsci1016/Arduino-EasyTransfer
//
// ===================================================================================================================================================================================================== 
// =====================================================================================================================================================================================================
// Update these as necessary to match your setup
//
//----------------------
// Begin Define Pins
//----------------------
#define enablePin 31          // Pin that provides power to motor driver enable pins
#define enablePinDome 29      // Pin that provides power to Dome motor driver enable pin
#define S2SpotPin A0          // Pin connected to side tilt potentiometer 
#define readpin 34            // Pin connected to ACT on soundboard
#define soundpin1 26          // Connected to sound pin 0
#define soundpin2 28          // Connected to sound pin 1
#define soundpin3 30          // Connected to sound pin 2
#define soundpin4 32          // Connected to sound pin 3
#define soundpin5 46          // Connected to sound pin 4
#define soundpin6 44          // Connected to sound pin 5
#define fadePin A2            // Connected to + of one channel on sound board(use resistor to ground)
#define BTstatePin 33         // Connected to state pin on BT Module
#define domeTiltPotPin A1     // Connected to Potentiometer on the dome tilt mast
#define easeDome 20           // Lower number means more easing when spinning
#define easeDomeTilt .8       // Lower number means more easing when moving forward and back a.k.a. slower
#define domeSpinPot A4        // Pin used to monitor dome spin potentiometer
#define battMonitor A3        // Pin used to monitor battery voltage
#define outputVoltage 5.2     // This is the output voltage from the Buck Converter powering the Arduino
#define drivePWM1 11          // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define drivePWM2 12          // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define s2sPWM1 7             // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define s2sPWM2 8             // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeTiltPWM1 5        // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeTiltPWM2 6        // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeSpinPWM1 9        // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define domeSpinPWM2 10       // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed
#define flywheelSpinPWM1 3    // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed 
#define flywheelSpinPWM2 4    // PWM Pin for movement, swap the pin numbers on this axis if axis is reversed 
#define resistor1 151000      // Larger resistor used on voltage divider to read battery level
#define resistor2 82000       // Smaller resistor used on voltage divider to read battery level
#define flywheelEase 3        // Speed in which flywheel will increase/decrease during gradual movements
#define S2SEase 2             // speed in which side to side moves. Higher number equates to faster movement
#define MaxDomeTiltAngle 17   // Maximum angle in which the dome will tilt. **  Max is 25  **

//#define TiltDomeForwardWhenDriving      // Uncomment this if you want to tilt the dome forward when driving. 

//---------------------------------
// Joystick reverse
//---------------------------------
//#define reverseDrive                    // Uncomment if your drive joystick is reversed
//#define reverseDomeTilt                 // Uncomment if your dome tilt joystick is reversed
//#define reverseS2S                      // Uncomment if your side to side joystick is reversed
//#define reverseDomeSpin                 // Uncomment if your dome spin joystick is reversed
//#define reverseFlywheel                 // Uncomment if your flywheel joystick is reversed
#define reversePitch                    // reverse Pitch. Test this by moving the drive by hand; the weight/stabilization should move WITH you, NOT AGAINST. 
//#define reverseRoll                     // reverse Roll. Test this by moving the drive by hand; the weight/stabilization should move WITH you, NOT AGAINST. 
//#define reverseDomeTiltPot
//#define reverseDomeSpinPot
//#define reverseS2SPot

//---------------------------------
// Debug scripts. Uncomment to run
//---------------------------------
//#define printRemote              // Uncomment to see values passed from controller
//#define debugS2S                 // Uncomment to see Side tilt variables, PID values, ETC.
//#define debugDrive               // Uncomment to see main drive variables, PID values, ETC.
//#define debugDomeTilt            // Uncomment to see Dome tilt variables, PID values, ETC.
//#define debugdomeRotation        // Uncomment to see Dome rotation variables, PID values, ETC.
//#define debugPSI                 // Uncomment to see PSI values.
//#define printbodyBatt            // Uncomment to see battery level 
//#define printYPR                 // Uncomment to see Yaw, Pitch, and Roll
//#define printDome                // Uncomment to see the Dome's Yaw
//#define printOffsets             // Uncomment to see the offsets
//#define debugRSelectMillis
//#define printOutputs
//#define printSoundPins           // Uncomment to see Sound Pins
//#define debugFlywheelSpin        // Uncomment to see flywheel values
//#define checkEN                  // Uncomment to see if controller is sending motorEnable value

//-----------------------
// Libraries to include
//-----------------------
#include <EEPROMex.h>   
#include "Arduino.h"
#include <EasyTransfer.h>
#include <PID_v1.h>        

//------------------------------------------------
// Create Objects for EasyTransfer on Serial Port
//------------------------------------------------
EasyTransfer RecDome; 
EasyTransfer RecRemote;
EasyTransfer SendDome;
EasyTransfer RecIMU; 

//----------------------------------------------------
// Structure of Data to recieved from the XBOX Remote
//----------------------------------------------------
struct RECEIVE_DATA_STRUCTURE_DOME{
  double domeBatt = 0.00;
};

struct RECEIVE_DATA_STRUCTURE_REMOTE{
  int ch1;                //right joystick up/down
  int ch2;                //right joystick left/right
  int ch3;                //left joystick up/down
  int ch4 = 256;          //left joystick left/right
  int ch5;                //flywheel
  int but1 = 1;           //left select
  int but2 = 1;           //left button 1
  int but3 = 1;           //left button 2
  int but4 = 1;           //left button 3 
  int but5 = 0;           //fwd/rev
  int but6 = 1;           //right button 1
  int but7 = 1;           //right button 2
  int but8 = 1;           //right select
  int motorEnable = 1; 
};
struct SEND_DATA_STRUCTURE_DOME{
  int PSI = 0;
  int button4;
};
struct RECEIVE_DATA_STRUCTURE_IMU{
  float IMUloop;
  float pitch;
  float roll;
};
  float pitch;
  float roll;

RECEIVE_DATA_STRUCTURE_DOME recFromDome;
RECEIVE_DATA_STRUCTURE_REMOTE recFromRemote;
SEND_DATA_STRUCTURE_DOME sendToDome;
RECEIVE_DATA_STRUCTURE_IMU recIMUData;

//-----------------------
// Variables Definition
//-----------------------
int ch4Servo;           //left joystick left/right when using servo mode
int currentDomeSpeed;
int domeRotation;
int fadeVal = 0; 
int readPinState = 1; 
int playSound;
int soundPins[] = {soundpin1, soundpin2, soundpin3, soundpin4};
int randSoundPin;
int soundState;
int but6State = 0;
int musicState;
int autoDisableState;
unsigned long soundMillis;
unsigned long musicStateMillis = 0;
unsigned long but6LastDebounceTime = 0;
unsigned long but6DebounceDelay = 50;
unsigned long bodyStatusTime = 0;
unsigned long autoDisableMotorsMillis = 0;
int autoDisableDoubleCheck;
unsigned long autoDisableDoubleCheckMillis = 0;
int autoDisable;
unsigned long lastLoopMillis;
float lastIMUloop;
int MiniStatus;
int flywheelRotation;
int SaveToEEPROM;
float R1 = resistor1; 
float R2 = resistor2; 
int joystickDrive;
int ch5PWM;
int lastDirection;
int driveSpeed = 55;
int driveAccel;

// the speedArray is used to create an S curve for the 'throttle' of bb8
int speedArray[] = {0,1,1,1,1,1,1,2,2,2,2,2,2,3,3,3,3,4,4,4,5,5,5,5,6,6,7,7,8,8,9,
9,10,10,11,12,12,13,13,14,15,16,16,17,18,19,19,20,21,22,23,24,25,26,26,27,28,29,30,
31,32,33,33,34,35,36,37,37,38,39,40,40,41,42,42,43,44,44,45,45,46,46,47,47,48,48,49,
49,50,50,50,51,51,51,52,52,52,52,53,53,53,53,54,54,54,54,54,55,55,55,55,55};

int joystickS2S;
int joystickDome;
double domeTiltOffset;
int domeTiltPot;
int domeSpinOffset;
int but1State;
unsigned long but1Millis;
int servoMode;
float countdown;
int domeServo = 0;
int S2Spot;
int BTstate = 0;
int speedDomeTilt = 0;

//---------------------------------
// Begin PID Loops
//---------------------------------
//PID1 is for the side to side tilt
double Pk1 = 13;  
double Ik1 = 0;
double Dk1 = 0.3;
double Setpoint1, Input1, Output1, Output1a;    

PID PID1(&Input1, &Output1, &Setpoint1, Pk1, Ik1 , Dk1, DIRECT);   
 
//PID2 is for side to side stability
double Pk2 = .5; 
double Ik2 = 0;
double Dk2 = .01;
double Setpoint2, Input2, Output2, Output2a;    

PID PID2(&Input2, &Output2, &Setpoint2, Pk2, Ik2 , Dk2, DIRECT);    // PID Setup - S2S stability   

//PID3 is for the main drive
double Pk3 = 5; 
double Ik3 = 0;
double Dk3 = 0;
double Setpoint3, Input3, Output3, Output3a;    

PID PID3(&Input3, &Output3, &Setpoint3, Pk3, Ik3 , Dk3, DIRECT);    // Main drive motor

//PID4 is for dome tilt fwd/back
double Pk4 = 6;  
double Ik4 = 0;
double Dk4 = 0.05;
double Setpoint4, Input4, Output4, Output4a;    

PID PID4(&Input4, &Output4, &Setpoint4, Pk4, Ik4 , Dk4, DIRECT);   

double Setpoint5a;

//PID5 is for the dome spin servo
double Kp5=4, Ki5=0, Kd5=0;
double Setpoint5, Input5, Output5, Output5a;

PID PID5(&Input5, &Output5, &Setpoint5, Kp5, Ki5, Kd5, DIRECT);     

const byte numChars = 40;
char receivedChars[numChars];
char tempChars[numChars];        // temporary array for use when parsing

boolean newData = false;

const byte numCharsD = 32;
char receivedCharsD[numCharsD];
char tempCharsD[numCharsD];        // temporary array for use when parsing

char textFromDome[numChars] = {0};

boolean newDataD = false;

long domeInterval = 1000;
long domeServoMillis;
long setCalibMillis;

float pitchOffset;
float rollOffset;
int potOffsetS2S;
int domeTiltPotOffset;

int bodyCalibState = 0;

// ================================================================
// ===                      INITIAL SETUP                       ===
// ================================================================

void setup() {

    Serial.begin(115200);      // Pins located in RX/TX on Mega
    Serial1.begin(115200);     // Pins located in RX1/TX1 on Mega
    Serial2.begin(115200);     // Pins located in RX2/TX2 on Mega
    Serial3.begin(115200);     // Pins located in RX3/TX3 on Mega 
    
    RecDome.begin(details(recFromDome), &Serial3);
    RecRemote.begin(details(recFromRemote), &Serial1);
    SendDome.begin(details(sendToDome), &Serial3);
    RecIMU.begin(details(recIMUData), &Serial2);

    pinMode(enablePin, OUTPUT);  // enable pin
    pinMode(enablePinDome, OUTPUT);  // enable pin for dome spin
    pinMode(BTstatePin, INPUT);  // BT paired status

    pinMode(readpin, INPUT_PULLUP); // read stat of Act on Soundboard
    pinMode(soundpin1, OUTPUT); // play sound from pin 0 on Soundboard
    pinMode(soundpin2, OUTPUT); // play sound from pin 1 on Soundboard
    pinMode(soundpin3, OUTPUT); // play sound from pin 2 on Soundboard
    pinMode(soundpin4, OUTPUT); // play sound from pin 3 on Soundboard
    pinMode(soundpin5, OUTPUT); // play sound from pin 4 on Soundboard
    pinMode(soundpin6, OUTPUT); // play sound from pin 4 on Soundboard

    digitalWrite(soundpin6, HIGH);
    digitalWrite(soundpin5, HIGH);
    digitalWrite(soundpin4, HIGH);
    digitalWrite(soundpin3, HIGH);
    digitalWrite(soundpin2, HIGH);
    digitalWrite(soundpin1, HIGH);
    
    //---------------------
    // PID Setup
    //---------------------
    PID1.SetMode(AUTOMATIC);              // PID Setup -  S2S SERVO     
    PID1.SetOutputLimits(-255, 255);
    PID1.SetSampleTime(15);

    PID2.SetMode(AUTOMATIC);              // PID Setup -  S2S Stability
    PID2.SetOutputLimits(-255, 255);
    PID2.SetSampleTime(15);

    PID3.SetMode(AUTOMATIC);              // PID Setup - main drive motor
    PID3.SetOutputLimits(-255, 255);
    PID3.SetSampleTime(15);

    PID4.SetMode(AUTOMATIC);              // PID Setup - dome tilt
    PID4.SetOutputLimits(-255, 255);
    PID4.SetSampleTime(15);

    PID5.SetMode(AUTOMATIC);
    PID5.SetOutputLimits(-255, 255);      // PID Setup - dome spin 'servo'
    PID5.SetSampleTime(15);

    //--------------------------
    // Read offsets from EEPROM
    //--------------------------
    pitchOffset = EEPROM.readFloat(0);
    rollOffset = EEPROM.readFloat(4);
    potOffsetS2S = EEPROM.readInt(8);
    domeTiltPotOffset = EEPROM.readInt(12);
    domeSpinOffset = EEPROM.readInt(16);

   // if (abs(rollOffset) + abs(pitchOffset) + abs(potOffsetS2S) + abs(domeTiltPotOffset) == 0 ){
      setOffsetsONLY();
   // }
    }

 //---------------------
 // LOOP, bruh!
 //---------------------
 void loop() {
  RecIMU.receiveData();

  if (millis() - lastLoopMillis >= 20){
          sendAndReceive();
          checkMiniTime();
          sounds();
          psiVal();
          readVin();
          BTenable();
          setDriveSpeed();
          bodyCalib();
          movement();
          domeCalib();
          lastLoopMillis = millis();
  }     
}

//-----------------------
// Begin Functions Code
//-----------------------
void sendAndReceive(){
  
  // Send and Receive all Data
  RecDome.receiveData();
  RecRemote.receiveData();
  SendDome.sendData();

  if(recIMUData.IMUloop != 0){
    #ifdef reversePitch
  pitch = recIMUData.pitch * -1;
    #else
  pitch = recIMUData.pitch;
    #endif
    
  #ifdef reverseRoll
    roll = recIMUData.roll *-1;
  #else 
    roll = recIMUData.roll;
  #endif
  }
}

//-----------------------
// IMU MPU6050 Stuff
//-----------------------
void checkMiniTime(){
  if (recIMUData.IMUloop == 1 && lastIMUloop >=980){
      lastIMUloop = 0;
  } else if (recIMUData.IMUloop <1 && lastIMUloop > 3){
    lastIMUloop = 0;
  }
  if (recIMUData.IMUloop > lastIMUloop){
      lastIMUloop = recIMUData.IMUloop;
  if (MiniStatus != 1){
    MiniStatus = 1;
  }
  } else if (recIMUData.IMUloop <= lastIMUloop && MiniStatus != 0){
    lastIMUloop++;
  }
  if (recIMUData.IMUloop - lastIMUloop < -20 && recIMUData.IMUloop - lastIMUloop > -800){
      MiniStatus = 0;
      lastIMUloop = 0;
      recIMUData.IMUloop = 0;
 }
  if (lastIMUloop >= 999){
      lastIMUloop = 0;
  }
}

//---------------------------
// Sounds
//---------------------------
void sounds(){
  
  //This plays the sound from the B Button when BB8 turns on
  if (recFromRemote.but2 == 1 && playSound == 0){   
    playSound = 1;
  }
  
  //This plays the music from the Y Button        
  if (recFromRemote.but3 == 1){  
    musicState = 1;
    musicStateMillis = millis();
    digitalWrite(soundpin6, LOW);
  } else if (recFromRemote.but3 == 0) {
    digitalWrite(soundpin6, HIGH);
    musicState = 0;
  }
  
  //This is the code for playing Random Sound when B Button is Pressed
  if(playSound == 1){
    randSoundPin = random(0, 5);
    digitalWrite((soundPins[randSoundPin]), LOW);
    soundMillis = millis();
    playSound = 2;
  } else if (playSound == 2 && (millis() - soundMillis > 200)){
    digitalWrite((soundPins[randSoundPin]), HIGH);
    playSound = 0;
  }
}

//---------------------------
// PSI Values
//---------------------------
void psiVal(){
  sendToDome.button4 = recFromRemote.but4;
  readPinState = digitalRead(readpin);
  fadeVal = map(analogRead(fadePin), 0, 30, 0, 255);
    if ((readPinState == 0) && (fadeVal > 40) && (musicState == 0)){ 
         sendToDome.PSI = constrain(fadeVal, 0, 255);
       } else {
         sendToDome.PSI = 0;
       }
    if(readPinState == 1 && musicState != 0 && (millis() - musicStateMillis >= 500)){
        musicState = 0;
      }
}
//---------------------------
// Read Voltage
//---------------------------
void readVin() {
     //sendToRemote.bodyBatt= ((analogRead(battMonitor) * outputVoltage) / 1024.0) / (R2/(R1+R2)); 
}
//---------------------------
// Bluetooth Enable
//---------------------------   
void BTenable() {
     BTstate = 1;  // Always keep BTstate as 1 when using XBox Controller  
        if(recFromRemote.motorEnable == 0 && BTstate == 1) {          //if motor enable switch is on and BT connected, turn on the motor drivers          
          autoDisableMotors();
          digitalWrite(enablePinDome, HIGH);
      }
        else if (recFromRemote.motorEnable == 1 || BTstate == 0) {    //if motor enable switch is off OR BT disconnected, turn off the motor drivers
          digitalWrite(enablePin, LOW);
          digitalWrite(enablePinDome, LOW);
          autoDisableState = 0;
          autoDisableDoubleCheck = 0;
      }
    }
//---------------------------
// Drive Speed
//---------------------------
void setDriveSpeed(){    
  if(recFromRemote.but6 == 1){
      driveSpeed = 55;
    } else if(recFromRemote.but6 == 2){
      driveSpeed = 75;
    } else if(recFromRemote.but6 == 3){
      driveSpeed = 110;
    }
  }
//---------------------------
// Body Calibration
//---------------------------
void bodyCalib() {          
  if (recFromRemote.but8 == 0 && recFromRemote.but7 == 1){
      timeBodyCalibration();
     } else if ((recFromRemote.but8 == 1 || recFromRemote.but7 == 0 || recFromRemote.motorEnable == 0) && bodyCalibState != 0){
      bodyCalibState = 0;
     }
  //if (sendToRemote.bodyStatus == 1){
      //waitForConfirmationToSetOffsets();
    //}
  }
//---------------------------
// Actual Movement
//---------------------------
void movement() {
  debugRoutines();
    if (SaveToEEPROM != 0){
        setOffsetsAndSaveToEEPROM();
  }
    //if (sendToRemote.bodyStatus == 2){
    //waitForConfirmationToSetDomeOffsets();   
  //}
    if (recFromRemote.motorEnable == 0 && BTstate == 1 && MiniStatus != 0){
        unsigned long currentMillis = millis();
          sideTilt();
          mainDrive();
          domeTilt();
          flywheelSpin();
         } else {
          turnOffAllTheThings();
          #ifdef checkEN
          Serial.print(" TurnOffAllTheThings ");
          Serial.print(" recFromRemote.motorEnable: "); Serial.print(recFromRemote.motorEnable);
          Serial.print(" BTstate: "); Serial.print(BTstate);
          Serial.print(" MiniStatus: "); Serial.print(MiniStatus);
          Serial.print(" recIMUData.IMUloop: "); Serial.print(recIMUData.IMUloop);
          Serial.print(" lastIMUloop: "); Serial.println(lastIMUloop);
          #endif        
         }
  } 
//---------------------------
// Dome Calibration
//---------------------------
void domeCalib(){
  if(but1State == 0 && recFromRemote.but1 == 0){
     but1State = 1;
     but1Millis = millis();
  }
  if(but1State == 1 && recFromRemote.but1 == 0 && (millis() - but1Millis > 3000)){
     //sendToRemote.bodyStatus = 2;
     but1State = 2;
  } else if (but1State == 1 && recFromRemote.but1 == 1 && (millis() - but1Millis < 3000)){
  if(servoMode == 1){
     servoMode = 0;
     but1State = 0;
   } else {
     servoMode = 1;
     but1State = 0;
   }
  }
  if(servoMode == 0 || autoDisable == 1 || recFromRemote.motorEnable == 1){
     domeSpin();
  } else if (servoMode == 1 && autoDisable == 0){
     domeSpinServo();
  } 
}
//---------------------------
// Main Drive Code
//---------------------------
 void mainDrive() {
  #ifdef reverseDrive
    joystickDrive = map(recFromRemote.ch1, 0,512, driveSpeed, (driveSpeed * -1));  //Read joystick - change -55/55 to adjust for speed. 
  #else
    joystickDrive = map(recFromRemote.ch1, 0,512,(driveSpeed * -1), driveSpeed);  //Read joystick - change -55/55 to adjust for speed. 
  #endif    
 
  // moves through speedArray to match joystick. speedArray is set up to create an 's curve' for increasing/decreasing speed
  if ((joystickDrive > driveAccel) && (driveAccel >= 0)){         
       driveAccel ++;
       Setpoint3 = speedArray[constrain(abs(driveAccel),0, 110)];
      } 
  else if ((joystickDrive < driveAccel) && (driveAccel >= 0)) {
            driveAccel --;
            Setpoint3 = speedArray[constrain(abs(driveAccel),0, 110)];
      }
  else if ((joystickDrive > driveAccel) && (driveAccel <= 0)){
            driveAccel ++;
            Setpoint3 = (speedArray[constrain(abs(driveAccel),0, 110)] * -1);
      } 
  else if ((joystickDrive < driveAccel) && (driveAccel <= 0)) {
            driveAccel --;
            Setpoint3 = (speedArray[constrain(abs(driveAccel),0, 110)] * -1);
      }
            Setpoint3 = constrain(Setpoint3, -55, 55);
            Input3 = (pitch + pitchOffset);// - domeOffset; 
            
            //domeTiltOffset used to keep the ball from rolling when dome is tilted front/back
            PID3.Compute();
            
  if (Output3 >= 2) {   //make BB8 roll
      Output3a = abs(Output3);
      analogWrite(drivePWM1, Output3a);   
      analogWrite(drivePWM2, 0);
      }
  else if (Output3 < -2) { 
           Output3a = abs(Output3);
           analogWrite(drivePWM2, Output3a);  
           analogWrite(drivePWM1, 0);
      }
    }
//-------------------------------------------------------------------------------------
// Side to Side Movement
//-------------------------------------------------------------------------------------
//s2s left joystick goes from 0(LEFT) to 512(RIGHT)  
//The IMU roll should go DOWN as it tilts to the right, and UP as it tilts to the left
//The side to side pot should go UP as the ball tilts left, and LOW as it tilts right
//-------------------------------------------------------------------------------------
 void sideTilt() {
  #ifdef reverseS2S
  joystickS2S = map(constrain(recFromRemote.ch2, 0 , 512), 0,512,25,-25); //- is  left, + is  right
  #else
  joystickS2S = map(constrain(recFromRemote.ch2, 0 , 512), 0,512,-25,25); //- is  left, + is  right
  #endif

  // Setpoint will increase/decrease by S2SEase each time the code runs until it matches the joystick. This slows the side to side movement.  
  if ((Setpoint2 > -S2SEase) && (Setpoint2 < S2SEase) && (joystickS2S == 0)){
       Setpoint2 = 0;
     }
  else if ((joystickS2S > Setpoint2) && (joystickS2S != Setpoint2)){
            Setpoint2+=S2SEase;  
     }
  else if ((joystickS2S < Setpoint2) && (joystickS2S != Setpoint2)){
            Setpoint2-=S2SEase;
     }
  #ifdef reverseS2SPot
    S2Spot = map(analogRead(S2SpotPin), 0, 1024, 135,-135);
  #else
    S2Spot = map(analogRead(S2SpotPin), 0, 1024, -135,135);
  #endif       
    Input2 = roll + rollOffset; 
    Setpoint2 = constrain(Setpoint2, -25,25);
    PID2.Compute();  //PID2 is used to control the 'servo' control of the side to side movement. 

    Input1  = S2Spot + potOffsetS2S;
    Setpoint1 = map(constrain(Output2, -25,25), -25,25, 25,-25);
    PID1.Compute();   //PID1 is for side to side stabilization
        
  if ((Output1 <= -1) && (Input1 > -25)) {
       Output1a = abs(Output1);
       analogWrite(s2sPWM2, Output1a);   
       analogWrite(s2sPWM1, 0);
     }
  else if ((Output1 >= 1) && (Input1 < 25)) { 
            Output1a = abs(Output1);
            analogWrite(s2sPWM1, Output1a);  
            analogWrite(s2sPWM2, 0);
     } else {
            analogWrite(s2sPWM2, 0);  
            analogWrite(s2sPWM1, 0);
            }
    }   
//--------------------------------------------------------------------
// Dome Tilt
//--------------------------------------------------------------------
//The joystick will go from 0(Forward) to 512(Back). 
//The pot will get HIGH as it moves back, and LOW as it moves forward
//--------------------------------------------------------------------
 void domeTilt(){   
  #ifdef reverseDomeTilt
    #define revDome1 
    #define revDome2 *-1
  #else
    #define revDome1 *-1
    #define revDome2 
  #endif

  //speedDomeTilt offsets the dome based on the main drive to tilt it in the direction of movement. 
  if (Setpoint3 < 3 && Setpoint3 > -3) {
      speedDomeTilt = 0;
  } else {
      speedDomeTilt = Output3 / 20;
  }

  #ifdef reverseDomeTiltPot
    domeTiltPot = (map(analogRead(domeTiltPotPin), 0, 1024, 135, -135) + domeTiltPotOffset);
  #else
    domeTiltPot = (map(analogRead(domeTiltPotPin), 0, 1024, -135, 135) + domeTiltPotOffset);
  #endif
  #ifdef TiltDomeForwardWhenDriving
    joystickDome = constrain(map(recFromRemote.ch3, 0,512,MaxDomeTiltAngle,-MaxDomeTiltAngle), MaxDomeTiltAngle revDome1, MaxDomeTiltAngle revDome2) - speedDomeTilt;   // Reading the stick for angle -40 to 40
  #else
    joystickDome = constrain(map(recFromRemote.ch3, 0,512,MaxDomeTiltAngle,-MaxDomeTiltAngle), MaxDomeTiltAngle revDome1 , MaxDomeTiltAngle revDome2);   // Reading the stick for angle -40 to 40
  #endif       
    Input4  = domeTiltPot + (pitch + pitchOffset);
  if ((Setpoint4 > -1) && (Setpoint4 < 1) && (joystickDome == 0)){
       Setpoint4 = 0;
     } 
  else if ((joystickDome > Setpoint4) && (joystickDome != Setpoint4)){
            Setpoint4 += easeDomeTilt;
     }
  else if ((joystickDome < Setpoint4) && (joystickDome != Setpoint4)){
            Setpoint4 -= easeDomeTilt;
     }
    Setpoint4 = constrain(Setpoint4, -MaxDomeTiltAngle,MaxDomeTiltAngle);
    PID4.Compute();      
         if (Output4 < -0 && domeTiltPot > -25) {
            Output4a = abs(Output4);
            analogWrite(domeTiltPWM2, Output4a);    
            analogWrite(domeTiltPWM1, 0);
            }
         else if (Output4 >= 0 && domeTiltPot < 25) { 
            Output4a = abs(Output4);
            analogWrite(domeTiltPWM1, Output4a);  
            analogWrite(domeTiltPWM2, 0);
            } 
         else {
            analogWrite(domeTiltPWM2, 0);  
            analogWrite(domeTiltPWM1, 0);
            }
    }
//---------------------------
// Dome Spin
//---------------------------
 void domeSpin() {
  #ifdef reverseDomeSpin
    domeRotation = map(recFromRemote.ch4, 0,512,255,-255);
  #else
    domeRotation = map(recFromRemote.ch4, 0,512,-255,255);
  #endif
       
  if (domeRotation < 3 && domeRotation > -3 && currentDomeSpeed > -15 && currentDomeSpeed < 15) {
      domeRotation = 0;
      currentDomeSpeed = 0;
     }
  if ((domeRotation > currentDomeSpeed) && (currentDomeSpeed >= 0)){
       currentDomeSpeed += easeDome ;
     } 
  else if ((domeRotation < currentDomeSpeed) && (currentDomeSpeed >= 0)){
            currentDomeSpeed -= easeDome ;
  } 
  else if ((domeRotation > currentDomeSpeed) && (currentDomeSpeed <= 0)){
            currentDomeSpeed += easeDome ;
  } 
  else if ((domeRotation < currentDomeSpeed) && (currentDomeSpeed <= 0)){
            currentDomeSpeed -= easeDome ;
  } 
    
  if ((currentDomeSpeed<=-20) && (BTstate ==1)){
       currentDomeSpeed = constrain(currentDomeSpeed,-255,255);
       analogWrite(domeSpinPWM2, 0);
       analogWrite(domeSpinPWM1, abs(currentDomeSpeed));
     }
     else if ((currentDomeSpeed>=20) && (BTstate == 1)){
               currentDomeSpeed = constrain(currentDomeSpeed,-255,255);
               analogWrite(domeSpinPWM1, 0);
               analogWrite(domeSpinPWM2, abs(currentDomeSpeed));
     }
     else {
       analogWrite(domeSpinPWM1, 0);
       analogWrite(domeSpinPWM2, 0);
    }   
 }

//---------------------------
// Flywheel Spin
//---------------------------
 void flywheelSpin() {
  #ifdef reverseFlywheel
    ch5PWM = constrain(map(recFromRemote.ch5, 0,512,255,-255),-255,255);
  #else
    ch5PWM = constrain(map(recFromRemote.ch5, 0,512,-255,255),-255,255);
  #endif        
    if(ch5PWM > -1 && ch5PWM < 35){
       ch5PWM = 0;
    } 
    else if(ch5PWM < 0 && ch5PWM > -35){
            ch5PWM = 0;
    } 
    else if(ch5PWM > 35){
            map(ch5PWM, 35, 255, 0, 255);
    } 
    else if(ch5PWM < -35){
            map(ch5PWM, -35, -255, 0, -255);
    }
    constrain(ch5PWM, -255, 255);
          
    if((ch5PWM < -240 && ((flywheelRotation > -30 && flywheelRotation < 30) || flywheelRotation > 240)) || ((ch5PWM > 240) && ((flywheelRotation > -30 && flywheelRotation < 30) || flywheelRotation < -240)))  {
    if(ch5PWM > 240){
      flywheelRotation = 255;
     }
     else if(ch5PWM < -240){
             flywheelRotation = -255;
      }
     } 
     else if(flywheelRotation < 0 && ch5PWM > 240){
             flywheelRotation = 255;
     }
     else if(flywheelRotation > 0 && ch5PWM < -240){
             flywheelRotation = 255;
     }
     else if(ch5PWM > flywheelRotation) {
             flywheelRotation+=flywheelEase;
            
     }
     else if(ch5PWM < flywheelRotation) {
              flywheelRotation-=flywheelEase;     
     }        
      
     constrain(flywheelRotation, -255, 255);
    if ((flywheelRotation < -10) && (BTstate == 1) && (recFromRemote.motorEnable == 0)){
         analogWrite(flywheelSpinPWM1, 0);
         analogWrite(flywheelSpinPWM2, abs(flywheelRotation));
    }
    else if ((flywheelRotation > 10) && (BTstate == 1) && (recFromRemote.motorEnable == 0)){
            analogWrite(flywheelSpinPWM2, 0);
            analogWrite(flywheelSpinPWM1, abs(flywheelRotation));
    }
    else {
            analogWrite(flywheelSpinPWM1, 0);
            analogWrite(flywheelSpinPWM2, 0);
    }  
 }

//------------------------------------------------------------------------------------------
// Disable Drive
//------------------------------------------------------------------------------------------
//disables all PIDS and movement. This is to avoid any sudden jerks when re-enabling motors
//------------------------------------------------------------------------------------------ 
void turnOffAllTheThings(){
  joystickS2S = 0;
  Input2 = 0;
  Setpoint2 = 0; 
  Output2 = 0;
  Input1 = 0;
  Setpoint1 = 0;
  Output1 = 0;
  joystickDrive = 0;
  driveAccel = 0;
  Input3 = 0;
  Setpoint3 = 0;
  Output3 = 0;
  joystickDome = 0;
  Input4 = 0;
  Setpoint4 = 0;
  Output4 = 0;
  flywheelRotation = 0;
  analogWrite(domeSpinPWM2, 0);  
  analogWrite(domeSpinPWM1, 0);       
}

//------------------------------------------------------------------------------------------
// Set dome rotation to Servo Mode
//------------------------------------------------------------------------------------------

void domeSpinServo() {    
  #ifndef reverseDomeRotation
    ch4Servo = map(recFromRemote.ch4, 0, 512, 70, -70);
  #else
    ch4Servo = map(recFromRemote.ch4, 0, 512, -70, 70);
  #endif
  #ifdef reverseDomeSpinPot
    if(recFromRemote.but5 == 1){
      Input5 = ((map(analogRead(domeSpinPot),0, 1023, -180, 180) + domeSpinOffset)-180);
    }else {
      Input5 = map(analogRead(domeSpinPot),0, 1023, -180, 180) + domeSpinOffset;
    }
  #else
    if(recFromRemote.but5 == 1){
      Input5 = ((map(analogRead(domeSpinPot),0, 1023, 180, -180) + domeSpinOffset)-180);
    }else {
      Input5 = map(analogRead(domeSpinPot),0, 1023, 180, -180) + domeSpinOffset;
    }
  #endif
    if (Input5 < -180){
      Input5 += 360;
    } else if (Input5 > 180){
      Input5 -= 360;
    } else {
      Input5 = Input5;
    }
    if ((Setpoint5 > -5) && (Setpoint5 < 5) && (ch4Servo == 0)){
          Setpoint5 = 0;
         }
         else if ((ch4Servo > Setpoint5) && (ch4Servo != Setpoint5)){
          Setpoint5+=5;  
         }
         else if ((ch4Servo < Setpoint5) && (ch4Servo != Setpoint5)){
          Setpoint5-=5;
         }
    constrain(Setpoint5, -70, 70);
    
    PID5.Compute();
    if (Output5 < -4) {
            Output5a = constrain(abs(Output5),0, 255);
            analogWrite(domeSpinPWM1, Output5a);     
            analogWrite(domeSpinPWM2, 0);
            }
    else if (Output5 > 4) { 
            Output5a = constrain(abs(Output5), 0, 255);
            analogWrite(domeSpinPWM2, Output5a);  
            analogWrite(domeSpinPWM1, 0);
            } 
    else {
            analogWrite(domeSpinPWM2, 0);  
            analogWrite(domeSpinPWM1, 0);
            }
}

//------------------------------------------------------------------------------------------
// Count how long right select is pressed
//------------------------------------------------------------------------------------------
  void timeBodyCalibration(){

    unsigned long currentMillisBodyCalib = millis();

    if (recFromRemote.but8 == 0 && recFromRemote.but7 == 1 && bodyCalibState == 0){
      setCalibMillis = millis();
      bodyCalibState = 1;
    }
    
    if (bodyCalibState == 1 && currentMillisBodyCalib - setCalibMillis >= 3000){
      //setOffsetsAndSaveToEEPROM();
      //sendToRemote.bodyStatus = 1;
      bodyCalibState = 0;
    }

      #ifdef debugRSelectMillis
      
      Serial.print(" currentMillisBodyCalib: ");
      Serial.print(currentMillisBodyCalib);
      
      #endif
    
  }

  void waitForConfirmationToSetOffsets(){
    countdown += .15;
    if (countdown > 10 && recFromRemote.but8 == 0 && recFromRemote.motorEnable == 1){
      //countdown = 0;
      SaveToEEPROM = 1;
      //sendToRemote.bodyStatus = 0;
    } else if (countdown >= 500){
      //sendToRemote.bodyStatus = 0;
      countdown = 0;
    }

  }

//------------------------------------------------------------------------------------------
// Set offsets and save them to EEPROM
//------------------------------------------------------------------------------------------
  void setOffsetsAndSaveToEEPROM(){
    if(SaveToEEPROM == 1){
      pitchOffset = pitch * -1;
      EEPROM.writeFloat(0,pitchOffset);
      SaveToEEPROM = 2;
    }else if(SaveToEEPROM == 2){
      rollOffset = roll * -1;
      EEPROM.writeFloat(4,rollOffset);
      SaveToEEPROM = 3;
    }else if(SaveToEEPROM == 3){
      potOffsetS2S = 0 - (map(analogRead(S2SpotPin), 0, 1024, -135,135));
      EEPROM.writeInt(8,potOffsetS2S);
      SaveToEEPROM = 4;
    }else if (SaveToEEPROM == 4){
      domeTiltPotOffset = 0 - (map(analogRead(domeTiltPotPin), 0, 1024, -135, 135));
      EEPROM.writeInt(12,domeTiltPotOffset);
      SaveToEEPROM = 0;
      //sendToRemote.bodyStatus = 0;
      countdown = 0;
      playSound = 1;
    }
  }
  void setDomeSpinOffset() {
    if(recFromRemote.but5 == 1){
      domeSpinOffset = 180 - map(analogRead(domeSpinPot),0, 1023, 180, -180);
    }else{
      domeSpinOffset = 0 - map(analogRead(domeSpinPot),0, 1023, 180, -180);
    }
    EEPROM.writeInt(16,domeSpinOffset);
   // delay(200);
    //sendToRemote.bodyStatus = 0;
    playSound = 1; 
  }

//------------------------------------------------------------------------------------------
// Set offsets ONLY; this is used if nothing is stored in EEPROM
//------------------------------------------------------------------------------------------
  void setOffsetsONLY(){ 
    pitchOffset = 0 - pitch;
    rollOffset = 0 - roll;
    potOffsetS2S = 0 - (map(analogRead(S2SpotPin), 0, 1024, -135,135));
    domeTiltPotOffset = 0 - (map(analogRead(domeTiltPotPin), 0, 1024, -135, 135));
    //delay(200);
  }

//------------------------------------------------------------------------------------------
// Set Dome Offsets
//------------------------------------------------------------------------------------------
  void waitForConfirmationToSetDomeOffsets(){
    if(servoMode == 1){
      servoMode = 0;
    }
    countdown += .15;
    if (countdown > 5 && recFromRemote.but8 == 0 && recFromRemote.motorEnable == 1){
      setDomeSpinOffset();
      //sendToRemote.bodyStatus = 0;
      but1State = 0;
    } else if (countdown >= 250){
      //sendToRemote.bodyStatus = 0;
      countdown = 0;
      but1State = 0;
    }
  }

//------------------------------------------------------------------------------------------
// Auto disable motors
//------------------------------------------------------------------------------------------
  void autoDisableMotors(){  
    if((joystickDrive > -2 && joystickDrive < 2) && (joystickS2S > -2 && joystickS2S < 2) && (joystickDome > -2 && joystickDome < 2) && (flywheelRotation < 25 && flywheelRotation > -25) && (recFromRemote.ch4 < 276 && recFromRemote.ch4 > 236) && (autoDisableState == 0)){
        autoDisableMotorsMillis = millis();
        autoDisableState = 1;      
    } else if(joystickDrive < -2 || joystickDrive > 2 || joystickS2S < -2 || joystickS2S > 2 || joystickDome < -2 || joystickDome > 2 || flywheelRotation > 30 || flywheelRotation < -30 || recFromRemote.ch4 > 276 || recFromRemote.ch4 < 236 || (lastDirection != recFromRemote.but5)){
        autoDisableState = 0;     
        digitalWrite(enablePin, HIGH); 
        autoDisableDoubleCheck = 0; 
        autoDisable = 0;
      if(recFromRemote.but5 != lastDirection){ 
        lastDirection = recFromRemote.but5;
      }
    }
            
    if(autoDisableState == 1 && (millis() - autoDisableMotorsMillis >= 3000) && Output1a < 25 && Output3a < 8){
        digitalWrite(enablePin, LOW);
        
        autoDisable = 1;
        
    } else if(Output1a > 50 || Output3a > 20){
        autoDisableState = 0;
        digitalWrite(enablePin, HIGH);
        autoDisableDoubleCheck = 0;  
        autoDisable = 0;    
    } else if((Output1a > 25 || Output3a > 8) && autoDisableDoubleCheck == 0){
        autoDisableDoubleCheckMillis = millis();
        autoDisableDoubleCheck = 1;
       
    } else if((autoDisableDoubleCheck == 1) && (millis() - autoDisableDoubleCheckMillis >= 100)){
        if(Output1a > 30 || Output3a > 8){ 
        autoDisableState = 0;
        digitalWrite(enablePin, HIGH);
        autoDisableDoubleCheck = 0;
        autoDisable = 0;
        }else{
          autoDisableDoubleCheck = 0;
        }
    } 
  }

//------------------------------------------------------------------------------------------
// DEBUG Scripts
//------------------------------------------------------------------------------------------
  void debugRoutines(){
    
       // To enable scripts, uncomment " #Define " at top of code
       
       #ifdef debugDrive

          Serial.print(F(" joystickDrive: "));
          Serial.print(joystickDrive);
          Serial.print(F(" accel: "));
          Serial.print(driveAccel);
          Serial.print(F(" SetDrive: "));
          Serial.print(Setpoint3);
          Serial.print(F(" InDrive: "));
          Serial.print(Input3);
          Serial.print(F(" OutDrive: "));
          Serial.print(Output3);

        #endif

        #ifdef debugS2S

          Serial.print(F(" joystickS2S: "));
          Serial.print(joystickS2S);
          Serial.print(F(" Roll: "));
          Serial.print(roll);
          Serial.print(F(" RollOffset: "));
          Serial.print(rollOffset);
          Serial.print(F(" S2SPot: "));
          Serial.print(S2Spot);
          Serial.print(F(" In2: "));
          Serial.print(Input2);
          Serial.print(F(" Set2: "));
          Serial.print(Setpoint2);
          Serial.print(F(" Out2/Set1: "));
          Serial.print(Output2);
          Serial.print(F(" In1: "));
          Serial.print(Input1);
          Serial.print(F(" Out1: "));
          Serial.println(Output1);

        #endif

        #ifdef debugDomeTilt
        
          Serial.print(F(" joystickDome: "));
          Serial.print(joystickDome);
          Serial.print(F(" In4 :"));
          Serial.print(Input4);
          Serial.print(F(" Set4 :"));
          Serial.print(Setpoint4);
          Serial.print(F(" Out4 :"));
          Serial.print(Output4);

        #endif

        #ifdef debugdomeRotation

          Serial.print(F(" domeRotation: "));
          Serial.print(domeRotation);
          Serial.print(F(" currentDomeSpeed: "));
          Serial.print(currentDomeSpeed);
          Serial.print(F(" ch4Servo: "));
          Serial.print(ch4Servo);
          Serial.print(F(" In5: "));
          Serial.print(Input5);
          Serial.print(F(" Set5: "));
          Serial.print(Setpoint5);
          Serial.print(F(" Out5: "));
          Serial.print(Output5);
          //Serial.print(F(" yawOffset: "));
          //Serial.print(yawOffset);
          Serial.print(F(" domeServo: "));
          Serial.print(domeServo);
          //Serial.print(F(" domeYaw: "));
          //Serial.print(recFromDome.domeYaw);
          //Serial.print(F(" yaw: "));
          //Serial.print(yaw);
          Serial.print(F(" pot: "));
          Serial.println(analogRead(domeSpinPot));
          
        
        #endif

        #ifdef debugPSI

          Serial.print(F(" readPinState: "));
          Serial.print(readPinState);
          Serial.print(F(" fadeVal: "));
          Serial.print(fadeVal);
          Serial.print(F(" PSI: "));
          Serial.println(sendToDome.PSI);

        #endif

        #ifdef printbodyBatt

          Serial.print(F(" Vin: "));
          Serial.print(sendToRemote.bodyBatt);

        #endif

        
        #ifdef printYPR
        
          //Serial.print(F(" Yaw: "));
          //Serial.print(yaw);
          Serial.print(F(" Roll: "));
          Serial.print(roll);
          Serial.print(F("  Pitch: "));
          Serial.println(pitch);

        #endif

        #ifdef printDome
         
          //Serial.print(F(" Dome Yaw: "));
          //Serial.print(recFromDome.domeYaw);
          Serial.print(F(" Dome Batt: "));
          Serial.print(recFromDome.domeBatt);
          Serial.print (F(" PSI: "));
          Serial.print (sendToDome.PSI);
          Serial.print (F(" But4: "));
          Serial.print (sendToDome.button4);

        #endif

        #ifdef printRemote

          Serial.print (F("  Remote: "));
          Serial.print (recFromRemote.ch1);
          Serial.print (" , ");
          Serial.print (recFromRemote.ch2);
          Serial.print (F(" , "));
          Serial.print (recFromRemote.ch3);
          Serial.print (F(" , "));
          Serial.print (recFromRemote.ch4);
          Serial.print (F(" , "));
          Serial.print (recFromRemote.ch5);
          Serial.print (F(" , "));
          Serial.print (recFromRemote.but1);
          Serial.print (F(" , "));
          Serial.print (recFromRemote.but2);
          Serial.print (F(" , "));
          Serial.print (recFromRemote.but3);
          Serial.print (F(" , "));
          Serial.print (recFromRemote.but4);  
          Serial.print (F(" , "));
          Serial.print (recFromRemote.but5);
          Serial.print (F(" , "));
          Serial.print (recFromRemote.but6);
          Serial.print (F(" , "));
          Serial.print (recFromRemote.but7);
          Serial.print (F(" , "));
          Serial.print (recFromRemote.but8);
          Serial.print (F(" , "));
          Serial.print (recFromRemote.motorEnable);
          Serial.print ('\n');

         #endif

         #ifdef printOffsets
          
          Serial.print(" pitchOffset: ");
          Serial.print(pitchOffset);
          Serial.print(" rollOffset: ");
          Serial.print(rollOffset);
          Serial.print(" potOffsetS2S: ");
          Serial.print(potOffsetS2S);
          Serial.print("domeTiltPotOffset: ");
          Serial.println(domeTiltPotOffset);
            
         #endif

         #ifdef debugRSelectMillis

          //Serial.print(" currentMillisBodyCalib: ");
          //Serial.print(currentMillisBodyCalib);
          Serial.print(" setCalibMillis: ");
          Serial.print(setCalibMillis);
          Serial.print(" motorEnable: ");
          Serial.print(recFromRemote.motorEnable);
          Serial.print(" bodyCalibState: ");
          Serial.print(bodyCalibState);
          Serial.print(" bodyStatus: ");
          Serial.print(sendToRemote.bodyStatus);
          Serial.print(" countdown: ");
          Serial.print(countdown);
            
         #endif

         #ifdef printOutputs

          Serial.print(F(" Out1: ")); Serial.print(Output1a);
          Serial.print(F(" Out2: ")); Serial.print(Output2a);
          Serial.print(F(" Out3: ")); Serial.print(Output3a);
          Serial.print(F(" Out4: ")); Serial.println(Output4a);

         #endif

         #ifdef printSoundPins

          Serial.print(F(" Pin1: ")); Serial.print(digitalRead(soundpin1));
          Serial.print(F(" Pin2: ")); Serial.print(digitalRead(soundpin2));
          Serial.print(F(" Pin3: ")); Serial.print(digitalRead(soundpin3));
          Serial.print(F(" Pin4: ")); Serial.print(digitalRead(soundpin4));
          Serial.print(F(" Pin5: ")); Serial.print(digitalRead(soundpin5));
          Serial.print(F(" Pin6: ")); Serial.print(digitalRead(soundpin6));
          Serial.print(F(" soundState: ")); Serial.print(soundState);
          Serial.print(F(" readPinState: ")); Serial.print(digitalRead(readpin));
          Serial.print(F(" randSoundPin: ")); Serial.println(randSoundPin);

         #endif

         #ifdef debugFlywheelSpin

          Serial.print(F(" ch5: "));
          Serial.print(recFromRemote.ch5);
          Serial.print(F(" ch5PWM: "));
          Serial.print(ch5PWM);
          Serial.print(F(" flywheelRotation: "));
          Serial.println(flywheelRotation);

         #endif
  }





