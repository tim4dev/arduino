/* Sweep
 by BARRAGAN <http://barraganstudio.com> 
 This example code is in the public domain.

 modified 8 Nov 2013
 by Scott Fitzgerald
 http://arduino.cc/en/Tutorial/Sweep
*/ 

#include <Servo.h> 
 
Servo myservo;  // create servo object to control a servo 
                // twelve servo objects can be created on most boards
 
int pos;

#define SERVO_DELAY    15
#define SERVO_POS_MAX  150
#define SERVO_POS_MIN  30
#define SERVO_POS_INC  30
 
void setup() 
{ 
  Serial.begin(9600);
  Serial.println("Servo test");
  myservo.attach(10);
//  myservo.attach(10, 1000, 2000);
//  myservo.write(SERVO_POS_MIN);
//  delay(1000);
} 


void turn(int pos) 
{
  Serial.println(pos);
  myservo.write(pos);
  delay(300);
} 
 
void loop() 
{ 
  turn(90);
  delay(2000);
  turn(40);
  turn(90);
  turn(140);

/*
  for(pos = SERVO_POS_MIN; pos <= SERVO_POS_MAX; pos += SERVO_POS_INC) // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(SERVO_DELAY * SERVO_POS_INC);       // waits 15ms for the servo to reach the position 
  } 
  
//  delay(2000);
  
  for(pos = SERVO_POS_MAX; pos >= SERVO_POS_MIN; pos -= SERVO_POS_INC)     // goes from 180 degrees to 0 degrees 
  {                                
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(SERVO_DELAY * SERVO_POS_INC);       // waits 15ms for the servo to reach the position 
  } 
*/
/*
  for(pos = 1000; pos <= 2000; pos += 150)
  {
    myservo.writeMicroseconds(pos); 
    delay(150);                       // waits 15ms for the servo to reach the position 
  } 
  
  delay(5000);
  
  for(pos = 2000; pos>=1000; pos-=150)     // goes from 180 degrees to 0 degrees 
  { 
    myservo.writeMicroseconds(pos);     
    delay(150);
  } 
*/
} 

