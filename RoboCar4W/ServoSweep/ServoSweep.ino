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
 
int pos = 0;    // variable to store the servo position 
 
void setup() 
{ 
  Serial.begin(9600);
  Serial.println("Servo test");
  myservo.attach(10);  // attaches the servo on pin 9 to the servo object 
} 
 
void loop() 
{ 
/*  Serial.println("set 0");
  myservo.write(0);
  delay(5000);
  
  Serial.println("set 90");
  myservo.write(90);
  delay(5000);
  
  Serial.println("set 180");
  myservo.write(180);
  delay(5000);
*/

  Serial.println("start test 1");
  for(pos = 0; pos <= 180; pos += 1) // goes from 0 degrees to 180 degrees 
  {                                  // in steps of 1 degree 
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
  Serial.println("start test 2");
  for(pos = 180; pos>=0; pos-=1)     // goes from 180 degrees to 0 degrees 
  {                                
    myservo.write(pos);              // tell servo to go to position in variable 'pos' 
    delay(15);                       // waits 15ms for the servo to reach the position 
  } 
  
} 

