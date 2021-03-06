#include "Encoder.h"

// Here we define our pins and constants

#define M1SPEED       9 // used to modify the motor 1 speed RIGHT WHEEL
#define M2SPEED       10 // used to modify the motoe 2 speed LEFT WHEEL
#define M1DIR         7 // used to modify motor 1 direction 
#define M2DIR         8 // used to modify motor 2 direction 
#define CPR           800 // counts per revolution 

Encoder M1Encoder(2,5); // motor 1 encoder pins (maybe change 7 to 5 becuase of direction pin)
Encoder M2Encoder(3,6); // motor 2 encoder pins 


// Here we have the variables in charge of recording old and new encoder values 
double oldPosition1 = 0;
double oldPosition2 = 0;
double newPosition1 = 0;
double newPosition2 = 0;


double errorDistance = 0; // used to calculate difference in target and current angular position
double errorAngle = 0;
double voltageDistance = 0;
double voltageAngle = 0;
double pwmOutDistance = 0;
double pwmOutAngle = 0;

double distance = 0.25; // distance between the wheels
double radius = 0.0725; // radius of a wheel 

int loopCounts = 0;

// here we set the desired distance in ft 
int desiredPosition = 3; // will be set to 3, 5, 7 ft

// here we set the variable to take care of ft to counts 
double rectifiedPosition;

double rectifiedAngle;

double desiredAngle = 180;

int state = 1;

int flagDistance;
int flagAngle;


// controller values 
double Kp = 0.05;

void setup() {
  pinMode(4, OUTPUT); // here we set the tri-state
  digitalWrite(4, HIGH);
  
  pinMode(M1DIR, OUTPUT); // RIGHT WHEEL 
  pinMode(M2DIR, OUTPUT); // LEFT WHEEL 

  digitalWrite(M1DIR,HIGH);
  digitalWrite(M2DIR, LOW); // this ensures we are going straight 
  
  pinMode(M1SPEED, OUTPUT); // motor 1 speed 
  pinMode(M2SPEED, OUTPUT); // motor 2 speed
  
  pinMode(12, INPUT); // status flag indicator 

  Serial.begin(9600); // baud rate for the serial monitor
  Serial.println("READY!");
  Serial.println("Demo 1 Distance");
}

void loop() { // throw in case statements in order to keep track of the experiment we want to run 
  newPosition1 = abs(M1Encoder.read()/4);
  newPosition2 = abs(M2Encoder.read()/4);
  //Serial.println(newPosition1);

  // tuned value for angle 
  rectifiedAngle = 8.2*desiredAngle; // here we calculate the number of counts per degree and multiply by degrees 
  //rectifiedAngle = 5.7971*desiredAngle; // here we calculate the number of counts per degree and multiply by degrees 
  
  //rectifiedPosition = (desiredPosition/1.505346)*CPR; // calculate the amount of counts we need to travel
  rectifiedPosition = (desiredPosition/1.6)*CPR;
  Serial.println(rectifiedPosition);
  switch (state){ // if we are at state = 1 we are turning, state = 2 we are going straight 
    
    case 1: // here we turn a desired amount 
    if(flagAngle == 0) {
      angleControl(rectifiedAngle);
    }
    break;
    
    case 2: // here we will traverse the desired position 
    if(flagDistance == 0) {
      distanceControl(rectifiedPosition);
    }
    break;
  }
}

// here we have the function which takes care of rotating a specified angle 
void angleControl(double targetAngle){
  // this keeps track of the direction of rotation 
  if (targetAngle > 0){ // with a positive angle we will have a left turn (right wheel turns only) 

    errorAngle = (targetAngle - newPosition1); // calculate the difference in our desired angle and current angle 
    //Serial.println(errorAngle);
    voltageAngle = Kp*errorAngle; // use Kp to scale down 
    pwmOutAngle = (voltageAngle/7.8) * 255; // normalize to voltage value and counts 

    pwmOutAngle = abs(pwmOutAngle);

    if (pwmOutAngle > 255) {
      pwmOutAngle = 255;
    }

    if (voltageAngle < 2){
      flagAngle = 1;
    }
    if (flagAngle == 1) {
      analogWrite(M2SPEED,0);
      analogWrite(M1SPEED, 0);
      state = 2;
      if (loopCounts == 0){
        delay(1000); // meant to delay after the angle is done so that we can take care of the angle nicely 
      }
      loopCounts = loopCounts + 1; // maybe change this back to normal 
    }
    else{
      analogWrite(M2SPEED,0);
      analogWrite(M1SPEED, 0.5*pwmOutAngle);
    }

  }
  else{ // with a negative angle we will have a right turn (left wheel turns only)
    
    errorAngle = (abs(targetAngle) - newPosition2); // calculate the difference in our desired angle and current angle 
    voltageAngle = Kp*errorAngle; // use Kp to scale down

    pwmOutAngle = (voltageAngle/7.8) * 255; // normalize to voltage value and counts 

    pwmOutAngle = abs(pwmOutAngle);

    if (pwmOutAngle > 255) {
      pwmOutAngle = 255;
    }

    if (voltageAngle < 2){
      flagAngle = 1;
    }
    if (flagAngle == 1) {
      analogWrite(M1SPEED, 0);
      analogWrite(M2SPEED,0);
      state = 2;
      
    }
    else{
      analogWrite(M1SPEED,0);
      analogWrite(M2SPEED, 0.5*pwmOutAngle);
    }
  }
}

// here we have the function which takes care of going a specified distance 
void distanceControl(double targetDistance){ // was previously set to int (maybe change back)
  // these direction setting ensure that we are going straight 
  if (desiredAngle > 0){
    errorDistance = (targetDistance - newPosition2); // ERROR IS THE DESIRED POSITION - ACTUAL POSITION
    //Serial.println(errorDistance);
  
    voltageDistance = Kp*errorDistance; // Kp scales down our voltage so we are not starting/stopping like crazy 
  
    pwmOutDistance = (voltageDistance / 7.8) * 255; // WE SCALE DOWN OUR VOLTAGE VALUE TO THE PWM 255 VALUE (UNITS OF BITS)
  
    pwmOutDistance = abs(pwmOutDistance);

    if(pwmOutDistance > 255) {
      pwmOutDistance = 255; // ASSIGNS MAX SPEED 
    }

    if (voltageDistance < 2){ // if we hit voltage to zero our flag is 1
      flagDistance = 1;
    }
    if (flagDistance == 1){ // if our flag is one we stop the robot 
      analogWrite(M1SPEED, 0); // WRITES THE SPEED OF THE MOTOR 
      analogWrite(M2SPEED, 0); // set M2 speed to 0.5 * max speed
    }
    else { // otherwise we set our desired speeds
      analogWrite(M1SPEED, 0.5*pwmOutDistance); // WRITES THE SPEED OF THE MOTOR 
      analogWrite(M2SPEED, 0.5*pwmOutDistance); // set M2 speed to 0.5 * max speed
    } 
  }
  else{
    errorDistance = (targetDistance - newPosition1); // ERROR IS THE DESIRED POSITION - ACTUAL POSITION
    //Serial.println(errorDistance);
    voltageDistance = Kp*errorDistance; // Kp scales down our voltage so we are not starting/stopping like crazy 
  
    pwmOutDistance = (voltageDistance / 7.8) * 255; // WE SCALE DOWN OUR VOLTAGE VALUE TO THE PWM 255 VALUE (UNITS OF BITS)
  
    pwmOutDistance = abs(pwmOutDistance);

    if(pwmOutDistance > 255) {
      pwmOutDistance = 255; // ASSIGNS MAX SPEED 
    }

    if (voltageDistance < 2){ // if we hit voltage to zero our flag is 1
      flagDistance = 1;
    }
    if (flagDistance == 1){ // if our flag is one we stop the robot 
      analogWrite(M1SPEED, 0); // WRITES THE SPEED OF THE MOTOR 
      analogWrite(M2SPEED, 0); // set M2 speed to 0.5 * max speed
    }
    else { // otherwise we set our desired speeds
      analogWrite(M1SPEED, 0.5*pwmOutDistance); // WRITES THE SPEED OF THE MOTOR 
      analogWrite(M2SPEED, 0.5*pwmOutDistance); // set M2 speed to 0.5 * max speed
    }
    
    
  }
}
