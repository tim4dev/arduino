// ------------------------------------------------------------------------------
// This code shows how to control two servos of the "pan-tilt-construction kit"
// available on eBay (see http://www.ebay.de/itm/161296853624).
// For connecting high power (or simply many) servos to the Arduino, see also:
// http://rcarduino.blogspot.de/2012/04/servo-problems-with-arduino-part-1.html
// ------------------------------------------------------------------------------
// Created by CodeFinder on 30th of December 2014.
#include <Servo.h>


#define min(a,b) ((a) < (b) ? (a) : (b))
#define max(a,b) ((a) > (b) ? (a) : (b))

Servo panServo, tiltServo;

#define PAN_SERVO_PIN   10 // "rotation"
//#define TILT_SERVO_PIN  6 // "moving up and down"

// Servo- / application-specific constants:
#define MOTION_DELAY       20
#define MIN_ANGLE_PAN       1
#define MAX_ANGLE_PAN     178
#define MIN_ANGLE_TILT      1
#define MAX_ANGLE_TILT    129
// With respect to the product (see above), these angles depend on how the kit is
// actually being assembled. You may need to adapt these values appropriately.

void setup()
{
	panServo.attach(PAN_SERVO_PIN);
//	tiltServo.attach(TILT_SERVO_PIN);
}

void loop()
{
	static const uint16_t MIN_ANGLE = min(MIN_ANGLE_PAN, MIN_ANGLE_TILT);
	static const uint16_t MAX_ANGLE = max(MAX_ANGLE_PAN, MAX_ANGLE_TILT);

	for (int i = MIN_ANGLE; i <= MAX_ANGLE; ++i) {
		panServo.write(map(i, MIN_ANGLE, MAX_ANGLE, MIN_ANGLE_PAN,  MAX_ANGLE_PAN));
//		tiltServo.write(map(i, MIN_ANGLE, MAX_ANGLE, MIN_ANGLE_TILT,  MAX_ANGLE_TILT));
		delay(MOTION_DELAY);
	}
	delay(1000);
	for (int i = MAX_ANGLE; i >= MIN_ANGLE; --i) {
		panServo.write(map(i, MIN_ANGLE, MAX_ANGLE, MIN_ANGLE_PAN,  MAX_ANGLE_PAN));
//		tiltServo.write(map(i, MIN_ANGLE, MAX_ANGLE, MIN_ANGLE_TILT,  MAX_ANGLE_TILT));
		delay(MOTION_DELAY);
	}
	delay(1000);
}

