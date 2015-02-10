/*
HC-SR04 Ultrasonic Module Distance Measuring
Working Voltage : 5V(DC)
Detection distance: 2cm--450cm
High precision: Up to 0.3cm
pinout: 1-VCC 2-trig(T) 3-echo(R) 4-GND

Время измерения (определено опытным путем):
максимум 240 мсек, если расстояние слишком велико (out of range)
минимум  1 мсек, если расстояние слишком мало
расстояние 1.5 м ~ 10 мсек
*/

/* эти пины останутся свободны после подключения Motor Shield */
/* пины для подключения HC-SR04 Ultrasonic Module Distance Measuring */
#define SONIC_PIN_TRIG 14 //13
#define SONIC_PIN_ECHO 15 //2
// Detection distance: 2cm--450cm
const int SONIC_DISTANCE_MAX = 450;
const int SONIC_DISTANCE_MIN = 2;

// Detection distance: 2cm--450cm
#define distanceMAX 450
#define distanceMIN 2

long duration, distance;

// для измерения промежутков времени
unsigned long time;



void setup() {
  Serial.begin(9600);
  pinMode(SONIC_PIN_TRIG, OUTPUT);
  pinMode(SONIC_PIN_ECHO, INPUT);
}

void diffTime(unsigned long time1)
{
  unsigned long opDuration;
  // сколько времени заняла вся операция
  opDuration = millis() - time1;
  Serial.print("opDuration ");
  Serial.print(opDuration);
  Serial.println("  milliseconds");
}  

void loop() {
  //time = millis();
  distance = measureDistance();
  Serial.println(distance);
  //diffTime(time);
  delay(1500);
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
