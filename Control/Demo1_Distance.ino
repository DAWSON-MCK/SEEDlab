
.#include "Encoder.h"

// Here we define our pins and constants

#define M1SPEED       9 // used to modify the motor 1 speed RIGHT WHEEL
#define M2SPEED       10 // used to modify the motoe 2 speed LEFT WHEEL
#define M1DIR         7 // used to modify motor 1 direction 
#define M2DIR         8 // used to modify motor 2 direction 
#define CPR           800 // counts per revolution 

Encoder M1Encoder(2,5); // motor 1 encoder pins 
Encoder M2Encoder(3,6); // motor 2 encoder pins 


// Here we have the variables in charge of recording old and new encoder values 
double newPosition1 = 0;

// utilized in the distance control 
double errorDistance = 0; // used to calculate difference in target and current angular position
double voltage = 0;
double pwmOut = 0;

// here we set the desired distance in ft 
int desiredPosition = 3; // will be set to 3, 5, 7 ft

// here we set the variable to take care of ft to counts 
double rectifiedPosition;

// flag utilized to take care of stopping the distance/angle functions
int flag = 0;

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

void loop() { // loop used to call the distance control function 
  newPosition1 = M1Encoder.read()/4; // scales down the couns by 4
  
  rectifiedPosition = (desiredPosition/1.505346)*CPR; // calculate the amount of counts we need to travel

  if (flag == 0) { // if we have not hit a point where our velocity goes to zero, continue 
    distanceControl(rectifiedPosition);
  }
}

void distanceControl(double target){ // was previously set to int (maybe change back)
  
  errorDistance = (target - newPosition1); // ERROR IS THE DESIRED POSITION - ACTUAL POSITION
  
  voltage = Kp*errorDistance; // Kp scales down our voltage so we are not starting/stopping like crazy 
  Serial.println(voltage);

  pwmOut = (voltage / 7.8) * 255; // WE SCALE DOWN OUR VOLTAGE VALUE TO THE PWM 255 VALUE (UNITS OF BITS)
  //Serial.println(pwmOut);
  pwmOut = abs(pwmOut);

  if(pwmOut > 255) {
    pwmOut = 255; // ASSIGNS MAX SPEED 
  }

  if (voltage < 2){ // if we hit voltage to zero our flag is 1
    flag = 1;
  }
  
  if (flag == 1){ // if our flag is one we stop the robot 
    analogWrite(M1SPEED, 0); // WRITES THE SPEED OF THE MOTOR 
    analogWrite(M2SPEED, 0); // set M2 speed to 0.5 * max speed
  }
  else { // otherwise we set our desired speeds 
    analogWrite(M1SPEED, 0.5*pwmOut); // WRITES THE SPEED OF THE MOTOR 
    analogWrite(M2SPEED, 0.5*pwmOut); // set M2 speed to 0.5 * max speed
  }
}
