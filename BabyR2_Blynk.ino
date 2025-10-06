#include <DFRobotDFPlayerMini.h>
#include <ESP32Servo.h>
#include <DFRobotDFPlayerMini.h>
#include <SoftwareSerial.h>
#include <HardwareSerial.h>

/*************************************************************
  Blynk is a platform with iOS and Android apps to control
  ESP32, Arduino, Raspberry Pi and the likes over the Internet.
  You can easily build mobile and web interfaces for any
  projects by simply dragging and dropping widgets.

    Downloads, docs, tutorials: https://www.blynk.io
    Sketch generator:           https://examples.blynk.cc
    Blynk community:            https://community.blynk.cc
    Follow us:                  https://www.fb.com/blynkapp
                                https://twitter.com/blynk_app

  Blynk library is licensed under MIT license
  This example code is in public domain.

 *************************************************************
  This example runs directly on ESP32 chip.

  NOTE: This requires ESP32 support package:
    https://github.com/espressif/arduino-esp32

  Please be sure to select the right ESP32 module
  in the Tools -> Board menu!

  Change WiFi ssid, pass, and Blynk auth token to run :)
  Feel free to apply it to any other example. It's simple!
 *************************************************************/

/* Comment this out to disable prints and save space */
#define BLYNK_PRINT Serial

/* Fill in information from Blynk Device Info here */
#define BLYNK_TEMPLATE_ID "TMPL22Z6oqEt8"
#define BLYNK_TEMPLATE_NAME "Baby R2"
#define BLYNK_AUTH_TOKEN "RVHZrQLyAzIgobbVHTdxTI_rgH33klAb"


#include <WiFi.h>
#include <WiFiClient.h>
#include <BlynkSimpleEsp32.h>

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "WeLovePink";
char pass[] = "IloveEleanor";

// Variables //
int xVal, yVal, headSpeed;
bool startup, headLeft, headRight, track1, track2, track3, effect;
double rampUpHead, rampUpBody;

Servo leftServo, rightServo, headServo;
const int leftServoPin = 25;
const int rightServoPin = 26;
const int headServoPin = 27;
const int rxPin = 16;
const int txPin = 17;
const int NEUTRAL = 1500;
const int CONTROLLER_NEUTRAL = 128;
const int NEUTRAL_ZONE = 20;
const int MIN_CONTROLLER = 0;
const int MAX_CONTROLLER = 256;

HardwareSerial soundfx(1);
DFRobotDFPlayerMini myDFPlayer;

//Blynk App inputs //
BLYNK_WRITE(V0)
{
  xVal = param[0].asInt();
  yVal = param[1].asInt();

  startup = true;
 }

BLYNK_WRITE(V1)
{ 
  int tracknr=param.asInt();

  if (tracknr>0)
  {
    Serial.print("track number is: ");
    Serial.println(tracknr);
    myDFPlayer.play(tracknr); // Make # of buttons to acommodate number of tracks you have, value off=0, on=tracknr //
  }
}

BLYNK_WRITE(V2)
{
  headLeft = param.asInt();
}

BLYNK_WRITE(V3)
{
  headRight = param.asInt();
}

BLYNK_WRITE(V4)
{
  headSpeed = param.asInt();
}

void setup()
{
  // Debug console
  Serial.begin(9600);
  soundfx.begin(9600, SERIAL_8N1, 16, 17);
  startup = false;
  headSpeed = 500;
  headLeft = false;
  headRight = false;

  // DFPlayerMini //
  Serial.println(F("DFRobot DFPlayer Mini Demo"));
  Serial.println(F("Initializing DFPlayer ... (May take 3~5 seconds)"));
  
  if (!myDFPlayer.begin(soundfx)) {  //Use softwareSerial to communicate with mp3.
    
    Serial.println(myDFPlayer.readType(),HEX);
    Serial.println(F("Unable to begin:"));
    Serial.println(F("1.Please recheck the connection!"));
    Serial.println(F("2.Please insert the SD card!"));
    while(true);
  }
  Serial.println(F("DFPlayer Mini online."));
  myDFPlayer.play(1);

  Blynk.begin(BLYNK_AUTH_TOKEN, ssid, pass);
}

void loop()
{
    int leftMotor, rightMotor;
  //leg motors
  int modifiedX = map(xVal, MIN_CONTROLLER, MAX_CONTROLLER, 0, 1000) - 500;
  int modifiedY = map(yVal, MIN_CONTROLLER, MAX_CONTROLLER, 0, 1000) - 500;

  if (modifiedX < 0) {
    leftMotor = NEUTRAL + modifiedY;
    rightMotor = NEUTRAL - ((modifiedY + 1) / abs(modifiedY + 1)) * 500;
  }
  else if (modifiedX > 0) {
    leftMotor = NEUTRAL + ((modifiedY + 1) / abs(modifiedY + 1)) * 500;
    rightMotor = NEUTRAL - modifiedY;
  }
  else {
    leftMotor = NEUTRAL + modifiedY;
    rightMotor = NEUTRAL - modifiedY;
  }

  // Sound Effects //
  if(track1 || track2 ||track3) {
    int effect; 
    if(track1) {
      effect = 1; // selects track 1 //
    }
    if(track2) {
      effect = 2; // selects track 2 //
    }
    if(track3) {
      effect = 3; // selects track 3 //
    }
    myDFPlayer.play(effect); // plays selected track //
    delay(5000);
      }

 
// Head Movement //
  if(headLeft || headRight) {
    int multiplier;
    if(headLeft) {
      multiplier = 1;
    }
    else {
      multiplier = -1;
    }
    
    headServo.attach(headServoPin);
    headServo.writeMicroseconds(NEUTRAL + map(headSpeed, 0, 1023, 0, 500) * multiplier * rampUpHead);

    if(rampUpHead < 1.0) {
      rampUpHead += 0.1;
    }
  }
  else {
    rampUpHead = 0.0;
    headServo.detach();
  }

  if (startup) {
    if(leftMotor == NEUTRAL && rightMotor == NEUTRAL) {
      rampUpBody = 0.0;
      leftServo.detach();
      rightServo.detach();
    }
    else {
      if(!leftServo.attached()) {
        leftServo.attach(leftServoPin);
      }
      if(!rightServo.attached()) {
        rightServo.attach(rightServoPin);
      }
      leftServo.writeMicroseconds(leftMotor);
      rightServo.writeMicroseconds(rightMotor);
    }
    // Serial.print(xVal);
    // Serial.println(" - xVal ");
    // Serial.print(yVal);
    // Serial.println(" - yVal");
    // Serial.print(modifiedX);
    // Serial.println(" - modified X");
    // Serial.print(modifiedY);
    // Serial.println(" - modified Y");
    // Serial.print(leftMotor);
    // Serial.println(" - left");
    // Serial.print(rightMotor);
    // Serial.println(" - right");
  }
  
  
  delay(50);
  Blynk.run();
}

