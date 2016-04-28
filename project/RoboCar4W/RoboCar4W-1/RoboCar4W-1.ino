/*
 * Arduino Robot Car 4 Wheel with Motor Shield, UltraSonic distance sensor
 * Arduino робот машина, 4-х колесный, с платой управления моторами, ультразвуковой измеритель расстояния
 * Версия без сервомотора
 * Copyright 2015 Yuriy Tim 
 * Полное описание создания на сайте http://tim4dev.com
 *
 * Используются библиотеки:
 * Sweep by BARRAGAN <http://barraganstudio.com> modified 8 Nov 2013 by Scott Fitzgerald http://arduino.cc/en/Tutorial/Sweep
 * Adafruit Motor shield library copyright Adafruit Industries LLC, 2009 
 */

#include <AFMotor.h>

#define VERSION "RoboCar4W ver.2015.02.05"

/*
 * Уровень отладки.
 * Чем больше, тем подробнее.
 * пока используются уровни:
 * Больше 1 - выдача отладочных сообщений
 * Больше 5 - реально моторы не включаются и дистанция не замеряется, а генерируется рандомно
 */
byte debug = 0;

// определяем моторы
AF_DCMotor motorFrontLeft(2);  //  передний левый
AF_DCMotor motorFrontRight(1); //  передний правый
AF_DCMotor motorRearLeft(3);   //  задний левый
AF_DCMotor motorRearRight(4);  //  задний правый

byte SPEED_CURRENT = 0; // текущая скорость моторов

/*
 * Виды поворотов
 */
const byte MOTOR_ROTATE_RIGHT = 0;  // вправо резкий разворот на месте (все левые колеса крутятся вперед, все правые - назад)
const byte MOTOR_TURN_RIGHT   = 1;  // вправо плавный поворот
const byte MOTOR_ROTATE_LEFT  = 2;  // влево резкий разворот на месте
const byte MOTOR_TURN_LEFT    = 3;  // влево плавный поворот
const byte MOTOR_TURN_BACK_RIGHT = 4; // поворот вправо задним ходом
const byte MOTOR_TURN_BACK_LEFT  = 5;

byte MOTOR_PREV_DIRECTION; // предыдущее выполненное направление движения

/*
 * Задержки для езды, поворотов на месте и плавных поворотов.
 * Подбираются экспериментально.
 */
const int DELAY_RUN    = 2;
const int DELAY_RUN_BACK = 50;
const int DELAY_ROTATE = 500;
const int DELAY_TURN   = 500;
const int DELAY_TURN_BACK = 500;

// в сантиметрах (distance threshold) Пороги расстояний до препятствия
// Если ближе, то резкий разворот на месте, иначе плавный доворот
const int DST_TRH_TURN = 28;
// Если ближе, то стоп и назад
const int DST_TRH_BACK = 15;

/* пины для подключения HC-SR04 Ultrasonic Module Distance Measuring 
 * 13, 2 цифровые пины
 * 14, 15 аналоговые пины A0 и A1 соответственно
 */
#define SONIC_PIN_TRIG 14 //13
#define SONIC_PIN_ECHO 15 //2
// Detection distance: 2cm--450cm
const int SONIC_DISTANCE_MAX = 450;
const int SONIC_DISTANCE_MIN = 2;



/******************************************
  Main program
******************************************/

void setup() {
  Serial.begin(9600);           // set up Serial library at 9600 bps
  if (debug > 1) Serial.println(VERSION);

  pinMode(SONIC_PIN_TRIG, OUTPUT);
  pinMode(SONIC_PIN_ECHO, INPUT);

  motorInit();
  if (debug  > 1) {
    delay(3000);
  }
}

void loop() {
  if (debug  > 1) Serial.println("\n*** new loop() start ***\n");
  // сравнить измеренные расстояния до препятствий
  // и определить направления движения
  int distance, ch;
  // замер расстояния
  distance = measureDistance();
  if (debug > 1) {
    Serial.print("distance = "); Serial.println(distance);
  }
  // препятствие так близко что надо ехать назад ?
  if ( distance <= DST_TRH_BACK ) {
    if (debug > 1) Serial.println("ALARM! Distance too small!!!");
    // стоп
    motorStop();
    if (debug  > 1) delay(1000);
    // ранее уже поворачивали задним ходом влево?
    if (MOTOR_TURN_BACK_LEFT == MOTOR_PREV_DIRECTION) {
      motorTurnBackRight();
    } else {
      motorTurnBackLeft();
    }
    if (debug  > 1) delay(1000);
    motorRunBack();
    return; // начать новый loop()
  }
  // определить направление поворота
  // прямо
  if ( distance > DST_TRH_TURN )   {
    motorRunForward();
  } else {
    motorStop();
    // направление поворота выбираем рандомно
    int rnd = random(1, 10);
    if (rnd > 5) {
      if (debug  > 1) delay(500);
      motorTurnLeft();
    } else {
      if (debug  > 1) delay(500);
      motorTurnRight();
    }
  }
}



/******************************************
  Functions
******************************************/

// инициализация моторов
void motorInit()  {
  if (debug > 1) Serial.println("motor Init");
  if (debug > 5) return;
  // turn on motor
  motorSetSpeed(190); // скорость мотора 0--255, реально меньше 100 не работает
  motorStop();
}

// движение вперед по прямой
void motorRunForward()  {
  if (debug > 1) Serial.println("Forward");
  if (debug > 5) return;
  motorFrontLeft.run(FORWARD);
  motorFrontRight.run(FORWARD);
  motorRearLeft.run(FORWARD);
  motorRearRight.run(FORWARD);
  delay(DELAY_RUN);
}

// движение назад по прямой
void motorRunBack()  {
  if (debug > 1) Serial.println("Backward");
  if (debug > 5) return;
  motorFrontLeft.run(BACKWARD);
  motorFrontRight.run(BACKWARD);
  motorRearLeft.run(BACKWARD);
  motorRearRight.run(BACKWARD);
  delay(DELAY_RUN_BACK);
}

// правый разворот на месте
void motorRotateRight()  {
  MOTOR_PREV_DIRECTION = MOTOR_ROTATE_RIGHT;
  if (debug > 1) Serial.println("Rotate R");
  if (debug > 5) return;
  motorFrontLeft.run(FORWARD);
  motorFrontRight.run(BACKWARD);
  motorRearLeft.run(FORWARD);
  motorRearRight.run(BACKWARD);
  delay(DELAY_ROTATE);
}

// правый плавный поворот (при движении вперед)
void motorTurnRight()  {
  MOTOR_PREV_DIRECTION = MOTOR_TURN_RIGHT;
  if (debug > 1) Serial.println("Turn R");
  if (debug > 5) return;
  motorFrontLeft.run(FORWARD);
  motorFrontRight.run(RELEASE);
  motorRearLeft.run(FORWARD);
  motorRearRight.run(RELEASE);
  delay(DELAY_TURN);
}

// правый плавный поворот (при движении назад)
void motorTurnBackRight()  {
  MOTOR_PREV_DIRECTION = MOTOR_TURN_BACK_RIGHT;
  if (debug > 1) Serial.println("Turn Back R");
  if (debug > 5) return;
  motorFrontLeft.run(BACKWARD);
  motorFrontRight.run(RELEASE);
  motorRearLeft.run(BACKWARD);
  motorRearRight.run(RELEASE);
  delay(DELAY_TURN_BACK);
}

// левый разворот на месте
void motorRotateLeft()  {
  MOTOR_PREV_DIRECTION = MOTOR_ROTATE_LEFT;
  if (debug > 1) Serial.println("Rotate L");
  if (debug > 5) return;
  motorFrontLeft.run(BACKWARD);
  motorFrontRight.run(FORWARD);
  motorRearLeft.run(BACKWARD);
  motorRearRight.run(FORWARD);
  delay(DELAY_ROTATE);
}

// левый плавный поворот (при движении вперед)
void motorTurnLeft()  {
  MOTOR_PREV_DIRECTION = MOTOR_TURN_LEFT;
  if (debug > 1) Serial.println("Turn L");
  if (debug > 5) return;
  motorFrontLeft.run(RELEASE);
  motorFrontRight.run(FORWARD);
  motorRearLeft.run(RELEASE);
  motorRearRight.run(FORWARD);
  delay(DELAY_TURN);
}

// левый плавный поворот (при движении назад)
void motorTurnBackLeft()  {
  MOTOR_PREV_DIRECTION = MOTOR_TURN_BACK_LEFT;
  if (debug > 1) Serial.println("Turn Back L");
  if (debug > 5) return;
  motorFrontLeft.run(RELEASE);
  motorFrontRight.run(BACKWARD);
  motorRearLeft.run(RELEASE);
  motorRearRight.run(BACKWARD);
  delay(DELAY_TURN_BACK);
}

// стоп резко
void motorStop()  {
  if (debug > 1) Serial.println("Stop");
  if (debug > 5) return;
  motorFrontLeft.run(RELEASE);
  motorFrontRight.run(RELEASE);
  motorRearLeft.run(RELEASE);
  motorRearRight.run(RELEASE);
}

// стоп плавно
void motorStopSlow()  {
  if (debug > 1) Serial.println("Stop slow");
  if (debug > 5) return;
  int speed;
  int diff = SPEED_CURRENT / 3; // сбрасываем скорость в 3 приема
  for (speed = SPEED_CURRENT; speed <= 0; speed -= diff) {
    motorSetSpeed(speed);
    delay(150);
  }
  motorStop(); // тормозим еще раз на всякий случай
}

// разгон плавно
void motorRunSlow()  {
  if (debug) Serial.println("Stop slow");
  if (debug > 5) return;
  int speed;
  int diff = (255 - SPEED_CURRENT) / 3; // набираем скорость в 3 приема
  for (speed = SPEED_CURRENT; speed > 255; speed += diff) {
    motorSetSpeed(speed);
    delay(150);
  }
  motorSetSpeed(255); // устанавливаем максималку еще раз на всякий случай
}

// установить скорость 0--255
void motorSetSpeed(int speed)  {
  // скорость мотора 0--255
  if (speed > 255)
    speed = 255;
  if (speed < 0)
    speed = 0;
  if (debug) {
    Serial.print("Motor set Speed = ");
    Serial.println(speed);
  }
  if (debug > 5) return;
  motorFrontLeft.setSpeed(speed);
  motorFrontRight.setSpeed(speed);
  motorRearLeft.setSpeed(speed);
  motorRearRight.setSpeed(speed);
  // запоминаем текущую скорость
  SPEED_CURRENT = speed;
}

// Возвращает расстояние до препятствия в сантиметрах
int measureDistance()  {
  if (debug > 5) return random(SONIC_DISTANCE_MIN, 50);
  long duration;
  int  distance;
  /* Для запуска передатчика нужно подать на Trig сигнал, длительностью 10мкс.
   * Передатчик который посылает 8 коротких импульсов с частотой 40kHz.
   * Приемник получает отраженный сигнал и на входе Echo генерируется сигнал,
   * длительность которого равна времени прохождения звукового сигнала.
   */
  digitalWrite(SONIC_PIN_TRIG, LOW); // инициализация перед замером
  delayMicroseconds(5); // 3
  digitalWrite(SONIC_PIN_TRIG, HIGH);
  delayMicroseconds(12); // 10
  digitalWrite(SONIC_PIN_TRIG, LOW);

  duration = pulseIn(SONIC_PIN_ECHO, HIGH);
  // Скорость звука 340 м/с или 29 микросекунд на сантиметр.
  // Звук идет вперед и возвращается назад, таким образом время нужно делить на два
  distance = duration / 58; // = microseconds / 29 / 2

  if (distance < SONIC_DISTANCE_MIN )  // out of range
    return SONIC_DISTANCE_MIN;
  if (distance > SONIC_DISTANCE_MAX )  // out of range
    return SONIC_DISTANCE_MAX;

  return distance;
}
