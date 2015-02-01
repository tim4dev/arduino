/*
 * Тестирование всех DC моторов, сервомотора и датчиков для
 * Arduino Robot Car 4 Wheel with UltraSonic distance sensor
 * Copyright 2015 Yuriy Tim http://tim4dev.com
 */
/* Sweep by BARRAGAN <http://barraganstudio.com> modified 8 Nov 2013 by Scott Fitzgerald http://arduino.cc/en/Tutorial/Sweep */
/* Adafruit Motor shield library copyright Adafruit Industries LLC, 2009 */

#include <AFMotor.h>
#include <Servo.h>

#define VERSION "Tests DC and Servo Motors, UltraSonic distance sensor for RoboCar4W"

/*
 * Сервомотор
 */
Servo servo;

// задержка для посылки команд в сервомотор
const int SERVO_DELAY  = 300;

/*
 * Измерения
 */
const int A_ANG[3] = {40, 90, 140}; // (array of angle) массив углов на которые должен поворачиваться серво для замера расстояний

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
  Serial.println(VERSION);
  Serial.println("");

  pinMode(SONIC_PIN_TRIG, OUTPUT);
  pinMode(SONIC_PIN_ECHO, INPUT);

  servo.attach(10);  // attaches the servo on pin 9 or 10 (крайний разъем)
  servo.write(0);
  delay(SERVO_DELAY);
}

void loop() {
  int i, k;
  // тест DC моторов
  for (i = 1; i < 5; i++) {
    Serial.print("DC motor Init : "); Serial.print(i);
    AF_DCMotor motor(i); // определяем DC мотор
    delay(10);

    Serial.print(" : FORWARD ");
    motor.setSpeed(255); // скорость мотора 0--255
    motor.run(FORWARD);
    delay(5000);

    Serial.print("BACKWARD ");
    motor.setSpeed(255); // скорость мотора 0--255
    motor.run(BACKWARD);
    delay(5000);

    Serial.println("STOP ");
    motor.run(RELEASE);
    delay(5000);
  }
  Serial.println("---------");

  // тест серво мотора и датчика расстояния
  int distance;

  Serial.println("Servo test (angle):");
  for (k = 0; k < 2; k++) {
    for (i = 0; i < 3; i++) {
      // поворот серво
        Serial.println(A_ANG[i]);
        servo.write( A_ANG[i] );
        delay(SERVO_DELAY);
        delay(3000);
    }
  }
  Serial.println("---------");

  Serial.println("UltraSonic distance sensor (cm)");
  servo.write( 90 );
  delay(SERVO_DELAY);
  for (k = 0; k < 2; k++) {
    // замер расстояния
    distance = measureDistance();
    Serial.println(distance);
    delay(5000);
  }
  Serial.println("------------------\n------------------\n");
}



/******************************************
  Functions
******************************************/

// Возвращает расстояние до препятствия в сантиметрах
int measureDistance()  {
  long duration;
  int  distance;
  /* Для запуска передатчика нужно подать на Trig сигнал, длительностью 10мкс.
   * Передатчик который посылает 8 коротких импульсов с частотой 40kHz.
   * Приемник получает отраженный сигнал и на входе Echo генерируется сигнал,
   * длительность которого равна времени прохождения звукового сигнала.
   */
  digitalWrite(SONIC_PIN_TRIG, LOW); // инициализация перед замером
  delayMicroseconds(3);
  digitalWrite(SONIC_PIN_TRIG, HIGH);
  delayMicroseconds(10);
  digitalWrite(SONIC_PIN_TRIG, LOW);

  duration = pulseIn(SONIC_PIN_ECHO, HIGH);
  // Скорость звука 340 м/с или 29 микросекунд на сантиметр.
  // Звук идет вперед и возвращается назад, таким образом время нужно делить на два
  distance = duration / 58; // = microseconds / 29 / 2
/*
  if (distance < SONIC_DISTANCE_MIN )  // out of range
    return SONIC_DISTANCE_MIN;
  if (distance > SONIC_DISTANCE_MAX )  // out of range
    return SONIC_DISTANCE_MAX;
*/
  return distance;
}
