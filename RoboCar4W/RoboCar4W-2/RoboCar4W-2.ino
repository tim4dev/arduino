/*
 * Arduino Robot Car 4 Wheel with Motor Shield, UltraSonic distance sensor, Bluetooth remote control
 * Arduino робот машина, 4-х колесный, с платой управления моторами, ультразвуковой измеритель расстояния,
 * с Bluetooth управлением (c) Andi.Co https://play.google.com/store/apps/details?id=braulio.calle.bluetoothRCcontroller
 *
 * Copyright 2015 Yuriy Tim
 * Полное описание создания на сайте http://tim4dev.com
 *
 * Используются библиотеки:
 * Sweep by BARRAGAN <http://barraganstudio.com> modified 8 Nov 2013 by Scott Fitzgerald http://arduino.cc/en/Tutorial/Sweep
 * Adafruit Motor shield library copyright Adafruit Industries LLC, 2009
 */

#include <AFMotor.h>

#define VERSION "RoboCar4W ver.2015.02.12"

/*
 * Уровень отладки.
 * Чем больше, тем подробнее.
 * пока используются уровни:
 * Больше 1 - выдача отладочных сообщений
 * Больше 5 - реально моторы не включаются и дистанция не замеряется, а генерируется рандомно
 */

// определяем моторы
AF_DCMotor motorFrontLeft(2);  //  передний левый
AF_DCMotor motorFrontRight(1); //  передний правый
AF_DCMotor motorRearLeft(3);   //  задний левый
AF_DCMotor motorRearRight(4);  //  задний правый

const byte SPEED_MIN = 100; // минимальная скорость моторов, если меньше - моторы остановятся. Подобрать экспериментально
const byte SPEED_MAX = 255; // максимальная скорость моторов
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


// в сантиметрах (distance threshold) Пороги расстояний до препятствия
// Если ближе, то стоп
const int DST_STOP = 20;
boolean SAFE_DISTANCE = true; // измерение дистанции и безопасное движение. Если впереди близко препятствие, то стоп
int distance = 0;

/* пины для подключения HC-SR04 Ultrasonic Module Distance Measuring
 * 13, 2 цифровые пины
 * 14, 15 аналоговые пины A0 и A1 соответственно
 */
#define SONIC_PIN_TRIG 14 //13
#define SONIC_PIN_ECHO 15 //2
// Detection distance: 2cm--450cm
const int SONIC_DISTANCE_MAX = 450;
const int SONIC_DISTANCE_MIN = 2;

// для управления по блютуз
char btCommand = 'S';
// счетчики для определения потери связи с блютуз
unsigned long btTimer0 = 2000;  //Stores the time (in millis since execution started)
unsigned long btTimer1 = 0;     //Stores the time when the last command was received from the phone




/******************************************
  Main program
******************************************/

void setup() {
  Serial.begin(9600);
  pinMode(SONIC_PIN_TRIG, OUTPUT);
  pinMode(SONIC_PIN_ECHO, INPUT);
  motorInit();
}


void loop() {
  if (Serial.available() > 0) {
    btTimer1 = millis();
    btCommand = Serial.read();
    //Serial.println(btCommand);
    switch (btCommand){
    case 'F':
      motorRunForward();
      break;
    case 'B':
      motorRunBack();
      break;
    case 'L':
      motorRotateLeft();
      break;
    case 'R':
      motorRotateRight();
      break;
    case 'S':
      motorStop();
      break;
    case 'I':  //FR
      motorTurnRight();
      break;
    case 'G':  //FL
      motorTurnLeft();
      break;
    case 'J':  //BR
      motorTurnBackRight();
      break;
    case 'H':  //BL
      motorTurnBackLeft();
      break;
    case 'W':  // демо программа (в оригинале Lights ON)
      roboCarDemo();
      break;
/*      case 'w':  //Lights OFF
        break;
      case 'U':  //Back ON
        break;
      case 'u':  //Back OFF
        break;
 */      case 'D':  //Everything OFF
      motorStop();
      break;
    case 'V': // ВКЛючить измерение дистанции и безопасное движение (в оригинале - гудок)
      SAFE_DISTANCE = true;
      break;
    case 'v': // ВЫКЛючить измерение дистанции и безопасное движение (в оригинале - гудок)
    SAFE_DISTANCE = false;
      break;
/*       case 'X': // аварийка
        break;
      case 'x': // аварийка
        break;
 */      default:  //Get SPEED_CURRENT
      if ( btCommand == 'q' ){
         motorSetSpeed(SPEED_MAX);  //Full SPEED
      } else{
        //Chars '0' - '9' have an integer equivalence of 48 - 57, accordingly.
        if ( (btCommand >= 48) && (btCommand <= 57) ) {
          // Subtracting 48 changes the range from 48-57 to 0-9.
          // Multiplying by 25 changes the range from 0-9 to 0-225.
          motorSetSpeed( SPEED_MIN + (btCommand - 48) * 15 );
        }
      } // else
    } // switch
  } // if (Serial.available() > 0)
  else{
    btTimer0 = millis();  //Get the current time (millis since execution started).
    //Check if it has been 500ms since we received last btCommand.
    if ((btTimer0 - btTimer1) > 500)   {
      //More tan 500ms have passed since last btCommand received, car is out of range.
      //Therefore stop the car and turn lights off.
      motorStop();
    }
  }
}



/******************************************
  Functions
******************************************/

// инициализация моторов
void motorInit()  {
  // turn on motor
  motorSetSpeed(100); // скорость мотора 0--255, реально меньше 100 не работает. См SPEED_MIN
  motorStop();
}

// безопасно ли ехать вперед?
boolean isSafeDistance() {
    if ( SAFE_DISTANCE )  {
        // замер расстояния
        distance = measureDistance();
        // препятствие так близко что надо ехать назад ?
        if ( distance < DST_STOP ) {
            return false;
        }
    } else {
        return true;
    }
}

// движение вперед по прямой
// если обнаружено препятствие, то машина останавливается и возвращается FALSE, иначе - TRUE
boolean motorRunForward()  {
  if ( !isSafeDistance() )  {
      motorStop();
      return false;
  }
  motorFrontLeft.run(FORWARD);
  motorFrontRight.run(FORWARD);
  motorRearLeft.run(FORWARD);
  motorRearRight.run(FORWARD);
  return true;
}

// движение назад по прямой
void motorRunBack()  {
  motorFrontLeft.run(BACKWARD);
  motorFrontRight.run(BACKWARD);
  motorRearLeft.run(BACKWARD);
  motorRearRight.run(BACKWARD);
}

// правый разворот на месте
void motorRotateRight()  {
  motorFrontLeft.run(FORWARD);
  motorFrontRight.run(BACKWARD);
  motorRearLeft.run(FORWARD);
  motorRearRight.run(BACKWARD);
}

// правый плавный поворот (при движении вперед)
// если обнаружено препятствие, то машина останавливается и возвращается FALSE, иначе - TRUE
boolean motorTurnRight()  {
  if ( !isSafeDistance() )  {
      motorStop();
      return false;
  }
  motorFrontLeft.run(FORWARD);
  motorFrontRight.run(RELEASE);
  motorRearLeft.run(FORWARD);
  motorRearRight.run(RELEASE);
  return true;
}

// правый плавный поворот (при движении назад)
void motorTurnBackRight()  {
  motorFrontLeft.run(BACKWARD);
  motorFrontRight.run(RELEASE);
  motorRearLeft.run(BACKWARD);
  motorRearRight.run(RELEASE);
}

// левый разворот на месте
void motorRotateLeft()  {
  motorFrontLeft.run(BACKWARD);
  motorFrontRight.run(FORWARD);
  motorRearLeft.run(BACKWARD);
  motorRearRight.run(FORWARD);
}

// левый плавный поворот (при движении вперед)
// если обнаружено препятствие, то машина останавливается и возвращается FALSE, иначе - TRUE
boolean motorTurnLeft()  {
  if ( !isSafeDistance() )  {
      motorStop();
      return false;
  }
  motorFrontLeft.run(RELEASE);
  motorFrontRight.run(FORWARD);
  motorRearLeft.run(RELEASE);
  motorRearRight.run(FORWARD);
  return true;
}

// левый плавный поворот (при движении назад)
void motorTurnBackLeft()  {
  motorFrontLeft.run(RELEASE);
  motorFrontRight.run(BACKWARD);
  motorRearLeft.run(RELEASE);
  motorRearRight.run(BACKWARD);
}

// стоп резко
void motorStop()  {
  motorFrontLeft.run(RELEASE);
  motorFrontRight.run(RELEASE);
  motorRearLeft.run(RELEASE);
  motorRearRight.run(RELEASE);
}

// установить скорость 0--255
// см SPEED_MIN
void motorSetSpeed(int speed)  {
  // скорость мотора 0--255
  if (speed > SPEED_MAX)
    speed = SPEED_MAX;
  if (speed < SPEED_MIN)
    speed = SPEED_MIN;
  motorFrontLeft.setSpeed(speed);
  motorFrontRight.setSpeed(speed);
  motorRearLeft.setSpeed(speed);
  motorRearRight.setSpeed(speed);
  // запоминаем текущую скорость
  SPEED_CURRENT = speed;
}

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

// правый или левый очень плавный поворот (при движении вперед)
// поворот за счет разных скоростей колес
// speedL - скорость левых  колес
// speedR - скорость правых колес
// если (speedL > speedR) то поворот будет вправо и наоборот
// если обнаружено препятствие, то машина останавливается и возвращается FALSE, иначе - TRUE
boolean motorSmoothTurn(byte speedL, byte speedR)  {
  if ( !isSafeDistance() )  {
      motorStop();
      return false;
  }
  // разные скорости
  // левые моторы
  motorFrontLeft.setSpeed(speedL);
  motorRearLeft.setSpeed(speedL);
  // правые моторы
  motorFrontRight.setSpeed(speedR);
  motorRearRight.setSpeed(speedR);
  // ехать вперед
  motorFrontLeft.run(FORWARD);
  motorFrontRight.run(FORWARD);
  motorRearLeft.run(FORWARD);
  motorRearRight.run(FORWARD);

  return true;
}

// Демо программа в автоматическом (неуправляемом) режиме.
// Выписывает фигуру, похожую на 8-ку
// При обнаружении препятствия машина останавливается.
void roboCarDemo()  {
  SAFE_DISTANCE = true; // безопасное движение - стоп при обнаружении препятствия

  // вперед
  motorSetSpeed(200);
  if ( !motorRunForward() ) // если обнаружено препятствие, то стоп
    return;
  delay(500); // временная задержка подбирается экспериментально

  // разворот влево
  if ( !motorTurnLeft() )  // если обнаружено препятствие, то стоп
    return;
  delay(2000);

  // вперед
  motorSetSpeed(130);
  if ( !motorRunForward() )   // если обнаружено препятствие, то стоп
    return;
  delay(500);

  // разворот вправо
  if ( !motorTurnRight() )  // если обнаружено препятствие, то стоп
    return;
  delay(2000);

  // вперед на максималке
  motorSetSpeed(SPEED_MAX);
  if ( !motorRunForward() ) // если обнаружено препятствие, то стоп
    return;
  delay(500);

  // стоп
  motorStop();
}
