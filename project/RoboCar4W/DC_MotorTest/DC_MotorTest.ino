// Adafruit Motor shield library
// copyright Adafruit Industries LLC, 2009
// this code is public domain, enjoy!

#include <AFMotor.h>

// определяем моторы
/*
AF_DCMotor motorFrontLeft(2);  //  передний левый
AF_DCMotor motorFrontRight(1); //  передний правый
AF_DCMotor motorRearLeft(3);   //  задний левый
AF_DCMotor motorRearRight(4);  //  задний правый
*/
AF_DCMotor motor(4);

byte SPEED_CURRENT = 0; // текущая скорость моторов

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println("Motor test!");
  // turn on motor
  motor.run(RELEASE);
}

void loop() {
  uint8_t i;

  delay(5000);
  // движение вперед по прямой
  motor.run(FORWARD);
  for (i=0; i<255; i++) {
    motor.setSpeed(i);  
    delay(20);
  }
  motor.run(RELEASE);
  delay(1000);

  motor.run(BACKWARD);
  for (i=255; i!=0; i--) {
    motor.setSpeed(i);
    delay(20);
  }
  motor.run(RELEASE);
}
