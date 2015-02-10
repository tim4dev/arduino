/*
 * Тест алгоритма управления движением для
 * Arduino Robot Car 4 Wheel with UltraSonic distance sensor
 * Copyright 2015 Yuriy Tim http://tim4dev.com
 */

/* Sweep by BARRAGAN <http://barraganstudio.com>
 modified 8 Nov 2013 by Scott Fitzgerald
 http://arduino.cc/en/Tutorial/Sweep
*/
/* Adafruit Motor shield library
   copyright Adafruit Industries LLC, 2009
*/

#include <AFMotor.h>
#include <Servo.h>

#define VERSION "*** Run test for RoboCar4W ***"

boolean debug = true; // для отладки

// определяем моторы
AF_DCMotor motorFrontLeft(2);  //  передний левый
AF_DCMotor motorFrontRight(1); //  передний правый
AF_DCMotor motorRearLeft(3);   //  задний левый
AF_DCMotor motorRearRight(4);  //  задний правый

byte SPEED_CURRENT = 0; // текущая скорость моторов

/*
 * Задержки для езды, поворотов на месте и плавных поворотов.
 * Подбираются экспериментально.
 */
const int DELAY_RUN    = 50;
const int DELAY_ROTATE = 500;
const int DELAY_TURN   = 500;

/*
 * Сервомотор
 */
Servo servo;

// задержка для посылки команд в сервомотор
const int SERVO_DELAY  = 300;

/*
 * Измерения
 */
const int  A_ANG[3]   = {140, 90, 40}; // (array of angle) массив углов на которые должен поворачиваться серво для замера расстояний
const char A_ANG_C[4] = "LFR"; // тот же массив, с указанием направления

// в сантиметрах (distance threshold) Пороги расстояний до препятствия
// Если ближе, то резкий разворот на месте, иначе плавный доворот
const int DST_TRH_TURN = 70;
// Если ближе, то стоп и назад
const int DST_TRH_BACK = 50;

/* пины для подключения HC-SR04 Ultrasonic Module Distance Measuring */
#define SONIC_PIN_TRIG 13
#define SONIC_PIN_ECHO 2
// Detection distance: 2cm--450cm
const int SONIC_DISTANCE_MAX = 100;
const int SONIC_DISTANCE_MIN = 2;



/******************************************
  Main program
******************************************/

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  Serial.println(VERSION);

  pinMode(SONIC_PIN_TRIG, OUTPUT);
  pinMode(SONIC_PIN_ECHO, INPUT);

  servo.attach(10);  // attaches the servo on pin 9 or 10 (крайний разъем)
  servo.write(0);
  motorInit();
}

void loop() {
  delay(5000);
  Serial.println("\n*** new loop() start ***\n");
  // сравнить измеренные расстояния до препятствий
  // и определить направления движения
  int i, distance, ch;
  int cur_dist = -1;  // расстояние
  int cur_angle = -1;  // угол
  char cur_angle_c;
  Serial.print("distance = ");
  for (i = 0; i < 3; i++) {
    // замер расстояния
    distance = measureDistance();
    Serial.print(distance); Serial.print("["); Serial.print( A_ANG_C[i] ); Serial.print("] ");

    // определение бОльшего расстояния до препятствия
    if ( distance > cur_dist )  {
        cur_dist    = distance;
        cur_angle   = A_ANG[i];
        cur_angle_c = A_ANG_C[i];
    }
  }
  Serial.println("");
  Serial.print("cur_dist  = "); Serial.println(cur_dist);
  Serial.print("cur_angle = "); /*Serial.print(cur_angle); Serial.print(" ");*/ Serial.println(cur_angle_c);
  Serial.println("");

  // препятствие прямо впереди и так близко что надо ехать назад ?
  if ( (90 == cur_angle) && (cur_dist <= DST_TRH_BACK) ) {
    Serial.println("ALARM! Distance too small!!!");
    // стоп
    motorStop();
    // случайно определяем направление заднего хода
    ch = random(1, 3);
    switch (ch) {
      case 1:
        motorTurnBackLeft();
        motorRunBack();
        break;
      case 2:
        motorRunBack();
        break;
      case 3:
        motorTurnBackRight();
        motorRunBack();
        break;
      default:
        motorRunBack();
    }
    return; // начать новый loop()
  }
  // определить направление поворота

  // прямо
  if ( (cur_angle >= 50) && (cur_angle <= 130) )   {
    motorRunForward();
    return; // начать новый loop()
  }
  // направо
  if ( (cur_angle >= 0) && (cur_angle < 50) )   {
    if (cur_dist < DST_TRH_TURN) {
      // резкий поворот (на месте)
      motorRotateRight();
    } else {
      // плавный поворот
      motorTurnRight();
    }
    // налево
  } else if ( (cur_angle > 130) && (cur_angle <= 180) )   {
    if (cur_dist < DST_TRH_TURN) {
      // резкий поворот (на месте)
      motorRotateLeft();
    } else {
      // плавный поворот
      motorTurnLeft();
    }
  }
  motorRunForward();
}



/******************************************
  Functions
******************************************/

// инициализация моторов
void motorInit()  {
  Serial.println("motor Init");
}

// движение вперед по прямой
void motorRunForward()  {
  Serial.println("Forward");
  delay(DELAY_RUN);
}

// движение назад по прямой
void motorRunBack()  {
  Serial.println("Backward");
  delay(DELAY_RUN);
}

// правый разворот на месте
void motorRotateRight()  {
  Serial.println("Rotate R");
  delay(DELAY_ROTATE);
}

// правый плавный поворот (при движении вперед)
void motorTurnRight()  {
  Serial.println("Turn R");
  delay(DELAY_TURN);
}

// правый плавный поворот (при движении назад)
void motorTurnBackRight()  {
  Serial.println("Turn Back R");
  delay(DELAY_TURN);
}

// левый разворот на месте
void motorRotateLeft()  {
  Serial.println("Rotate L");
  delay(DELAY_ROTATE);
}

// левый плавный поворот (при движении вперед)
void motorTurnLeft()  {
  Serial.println("Turn L");
  delay(DELAY_TURN);
}

// левый плавный поворот (при движении назад)
void motorTurnBackLeft()  {
  Serial.println("Turn Back L");
  delay(DELAY_TURN);
}

// стоп резко
void motorStop()  {
  Serial.println("Stop");
}

// стоп плавно
void motorStopSlow()  {
  int speed;
  int diff = SPEED_CURRENT / 3; // сбрасываем скорость в 3 приема
  Serial.println("Stop slow");
  for (speed = SPEED_CURRENT; speed <= 0; speed -= diff) {
    motorSetSpeed(speed);
    delay(150);
  }
  motorStop(); // тормозим еще раз на всякий случай
}

// разгон плавно
void motorRunSlow()  {
  int speed;
  int diff = (255 - SPEED_CURRENT) / 3; // набираем скорость в 3 приема
  Serial.println("Stop slow");
  for (speed = SPEED_CURRENT; speed > 255; speed += diff) {
    Serial.print("speed=");  Serial.println(speed);
    delay(150);
  }
  motorSetSpeed(255); // устанавливаем максималку еще раз на всякий случай
}

// установить скорость 0--255
void motorSetSpeed(int speed)  {
  Serial.print("Motor set Speed");
  // скорость мотора 0--255
  if (speed > 255)
    speed = 255;
  if (speed < 0)
    speed = 0;
  Serial.println(speed);
  // запоминаем текущую скорость
  SPEED_CURRENT = speed;
}

// Возвращает расстояние до препятствия в сантиметрах
int measureDistance()  {
  return random(SONIC_DISTANCE_MIN, SONIC_DISTANCE_MAX);
}
