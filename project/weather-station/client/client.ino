/*
 *
 * Weather Station RF-ESP-DHT-CLI
 * Код для КЛИЕНТА.
 *
 * Алгоритм работы:
 * Периодически клиенты (датчики) замеряют параметры и передают на сервер.
 * Сервер через WiFi передает данные на вебсервер.
 *
 * Полное описание на сайте http://tim4dev.com/tag/meteostantsiya/
 *
 */
/*
 * The MIT License (MIT)
 * Copyright (c) 2016 tim4dev.com
 *
 */

#define VERSION "Weather Station RF-ESP-DHT-CLI ver.05/2016.02.16"

#include "limits.h"

// ТОЛЬКО ДЛЯ ОТЛАДКИ. В ФИНАЛЬНОЙ ВЕРСИИ ЗАКОММЕНТИРОВАТЬ !
//#define DEBUG



/* Задержка главного цикла (измерение).
 * При запуске в первые 10 минут для проверки работоспособности блока проводятся укороченные циклы длительностью DELAY_1ST_LOOP
 */
#ifdef DEBUG
const int DELAY_LOOP     = 1;  // засыпаем на 8 сек
const int DELAY_1ST_LOOP = 1;
#else
const int DELAY_LOOP     = 600 / 8;  // 10 мин, засыпаем урывками по 8 сек
const int DELAY_1ST_LOOP = 120 / 8;  //  2 мин
#endif

// счетчик циклов
unsigned long LOOP_COUNT = 0;

// После включения идут первые LOOP_1ST укороченных циклов
// Проблема почему нельзя сделать через millis() в том, что при LowPower.powerDown время millis() тоже «засыпает»
const unsigned long LOOP_1ST = 10;



/* Подключение датчика температуры и влажности DHT22

    DHT22                 Arduino Pro Mini
    лицевая сторона
    слева направо
    VCC  ----------------- 3.3-5В (рекомендуется 5В, лучше внешнее питание)
    SDA  ----------------- D2     (см. ниже DHTPIN)
    NC   ----------------- не подсоединен
    GND  ----------------- GND
    Опционально:
    SDA --> 10K резистор --> VCC
 */
#include "DHT.h"
#define DHTPIN  2      // цифровой пин
#define DHTTYPE DHT22  // DHT 22  (AM2302), AM2321

// инициализация
DHT dht(DHTPIN, DHTTYPE);



/*
    Распиновка nRF24L01+ (смотреть сверху платы где чип, пины внизу)
    2 3,3V   4 CSN  6 MOSI  8 IRQ
    1 GND    3 CE   5 SCK   7 MISO

    Библиотека RadioHead : http://www.airspayce.com/mikem/arduino/RadioHead/

    Подключение nRF24L01+

     Arduino UNO           nRF24L01+
     Arduino Pro Mini
            3.3V-----------VCC 2 (лучше внешнее питание)
          pin D8-----------CE  3 (chip enable in)
       SS pin D10----------CSN 4 (chip select in)
      SCK pin D13----------SCK 5 (SPI clock in)
     MOSI pin D11----------SDI 6 (SPI Data in)
     MISO pin D12----------SDO 7 (SPI data out)
                           IRQ 8 (Interrupt output, не подсоединен)
              GND----------GND 1
Инициализация: NRF24 nrf24(8, 10);

 */

// http://www.airspayce.com/mikem/arduino/RadioHead/classRHReliableDatagram.html
#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <SPI.h>

// адрес сервера и клиента
#define SERVER_ADDRESS 10
#define CLIENT_ADDRESS 20  // ИЗМЕНИТЬ для другого экземпляра !!!
//  Номер радиоканала. Должен быть КАК И У СЕРВЕРА
#define RF_CHANNEL  73

// установка радио драйвера
#if defined(__AVR_ATmega328P__)
RH_NRF24 radio(8, 10);
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
RH_NRF24 radio(8, 53);
#else
RH_NRF24 radio;
#endif

// для управления надежной доставкой сообщений с помощью радио драйвера, установленного выше
RHReliableDatagram rfdata(radio, CLIENT_ADDRESS);

struct DHT_DATA_STRUCTURE {
  int temperature;
  int humidity;
  float voltage; // значение питающего напряжения
};
DHT_DATA_STRUCTURE     dhtData;

// Dont put this on the stack
uint8_t  buf[RH_NRF24_MAX_MESSAGE_LEN];



/*
 * Вольтметр.
 * Для измерения питающего напряжения батареи
 * подробнее здесь http://tim4dev.com/arduino-secret-true-voltmeter/
 */
#define V_PIN   1
// Количество измерений
#define COUNT_MEASURE 5

// Резисторы делителя напряжения, фактические значения
const float r1 = 100400; // 100K
const float r2 =  9960;  // 10K
// Эту константу необходимо откалибровать индивидуально
// как описано здесь http://tim4dev.com/arduino-secret-true-voltmeter/
const float typVbg = 1.082; // обычно в пределах 1.0 -- 1.2 В



/*
 * Lightweight low power library for Arduino
 * https://github.com/rocketscream/Low-Power
 *
 * Потребление Arduino Pro Mini :
 * - обычно 25mA
 * - при работе с DHT то же самое
 * - при радио передаче 38 mA
 * - при LowPower.idle 15 mA
 * - при LowPower.powerDown 7.5 mA
 */
#include "LowPower.h"





void setup()
{
    Serial.begin(57600);
    #ifdef DEBUG
    Serial.println("\n\n\n\n\n\n\n******* " + VERSION + " *******\n\n");
    #endif

    myError( "RF init",    rfdata.init() ); // по умолчанию устанавливается 2.402 GHz (канал 2), 2Mbps, 0dBm
    myError( "setChannel", radio.setChannel(RF_CHANNEL) ); // устанавливаем свой канал
    myError( "setRF",      radio.setRF(RH_NRF24::DataRate1Mbps, RH_NRF24::TransmitPower0dBm) );  // устанавливаем скорость 1Mbps
    rfdata.setRetries(5);    // число попыток передачи, по умолчанию 3
    rfdata.setTimeout(400);  // время таймаута передачи, по умолчанию 200мс

    #ifdef DEBUG
    Serial.print("dht.begin...");
    #endif

    dht.begin();

    #ifdef DEBUG
    Serial.println("OK");
    #endif

    // определение опорного напряжения
    analogReference(DEFAULT);  // DEFAULT INTERNAL использовать Vcc как AREF
    delay(100);
    analogWrite(V_PIN, 0);

    #ifdef DEBUG
    Serial.println("\n******* Setup complete *******\n\n");
    #endif
}





void loop()
{
    if (LOOP_COUNT < ULONG_MAX)
        LOOP_COUNT++;
    else
        LOOP_COUNT = 0;

    #ifdef DEBUG
    Serial.print("LOOP_COUNT = "); Serial.println(LOOP_COUNT);
    #endif
    if ( radio.statusRead() ) {

        delay(100);

        // делаем несколько замеров температуры и влажности
        // и находим среднее арифметическое
        myDhtRead();
        dhtData.voltage = getVoltage();

        #ifdef DEBUG
        Serial.print("Temperature:\t"); Serial.print(dhtData.temperature); Serial.print(" C\t");
        Serial.print("Humidity: ");     Serial.print(dhtData.humidity);    Serial.print(" %\t");
        Serial.print("Voltage: ");      Serial.print(dhtData.voltage);     Serial.println(" V");
        #endif

        // передача данных на сервер (с повторами, если потребуется)
        // по умолчанию: 3 повтора, с таймаутами (200мс) по retries*timeout
        // можно изменить с помощью setRetries() и setTimeout()
        #ifdef DEBUG
        if ( rfdata.sendtoWait((uint8_t*)&dhtData, sizeof(dhtData), SERVER_ADDRESS) )
        {
            uint8_t len = sizeof(buf);
            uint8_t from;

            // ждем ответа от сервера, либо истечения таймаута
            // необходимо вызывать эту фунцию достаточно часто, чтобы не пропустить сообщение от сервера
            if ( rfdata.recvfromAckTimeout(buf, &len, 3000, &from) )
            {
                Serial.print("From address : "); Serial.print(from);
                //Serial.print(" : ");               Serial.println((char*)buf);
            } else {
                Serial.println("No reply, is server running?");
            }
        } else
            Serial.println("sendtoWait failed");
        #else
        rfdata.sendtoWait((uint8_t*)&dhtData, sizeof(dhtData), SERVER_ADDRESS);
        #endif

        // переходим в режим Power Down до следующей передачи
        radio.sleep();
    }

    // засыпаем урывками по 8 сек
    myDelay();

    // будим радиомодуль
    delay(200);
    radio.available();
    delay(200);
}





/****************************************************************************
 * Функции
 ****************************************************************************/



void myDelay()
{
    int delay_cur;
    // После включения идут первые LOOP_1ST укороченных циклов
    if ( LOOP_COUNT < LOOP_1ST )
        delay_cur = DELAY_1ST_LOOP;
    else
        delay_cur = DELAY_LOOP;

    for ( int i = 0; i < delay_cur; i++ )  {
        LowPower.powerDown(SLEEP_8S, ADC_OFF, BOD_OFF); // засыпаем на 8 сек
    }
}



void myError(const String& msg, boolean res)
{
    #ifdef DEBUG
    if ( res )  {
        Serial.println(msg + " OK");
    } else {
        Serial.println(msg + " failed !");
    }
    #endif
}


// делаем несколько замеров температуры и влажности
// и находим среднее арифметическое
void myDhtRead()
{
    float curTemp = 0.0;
    float curHum  = 0.0;
    for (int i = 0; i < COUNT_MEASURE; i++) {
        curTemp = curTemp + dht.readTemperature();
        curHum  = curHum + dht.readHumidity();
        delay(50);
    }
    dhtData.temperature = round( curTemp / COUNT_MEASURE );
    dhtData.humidity    = round( curHum  / COUNT_MEASURE );
}



float getVoltage()  {
    byte i;
    float Vcc = readVcc();
    // считываем точное напряжение с A0, где будет находиться наш вольтметр с делителем напряжения
    float curVoltage = 0.0;
    for (i = 0; i < COUNT_MEASURE; i++) {
        curVoltage = curVoltage + analogRead(V_PIN);
        delay(20);
    }
    curVoltage = curVoltage / COUNT_MEASURE;

    float v  = (curVoltage * Vcc) / 1024.0;
    float v2 = v / (r2 / (r1 + r2));

    analogWrite(V_PIN, 0);
    delay(15);

    return v2;
}



float readVcc() {
    byte i;
    float result = 0.0;
    float tmp = 0.0;

    for (i = 0; i < 5; i++) {
        // Read 1.1V reference against AVcc
        // set the reference to Vcc and the measurement to the internal 1.1V reference
        #if defined(__AVR_ATmega32U4__) || defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
        ADMUX = _BV(REFS0) | _BV(MUX4) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
        #elif defined (__AVR_ATtiny24__) || defined(__AVR_ATtiny44__) || defined(__AVR_ATtiny84__)
        ADMUX = _BV(MUX5) | _BV(MUX0);
        #elif defined (__AVR_ATtiny25__) || defined(__AVR_ATtiny45__) || defined(__AVR_ATtiny85__)
        ADMUX = _BV(MUX3) | _BV(MUX2);
        #else
        // works on an Arduino 168 or 328
        ADMUX = _BV(REFS0) | _BV(MUX3) | _BV(MUX2) | _BV(MUX1);
        #endif

        delay(3); // Wait for Vref to settle
        ADCSRA |= _BV(ADSC); // Start conversion

        while (bit_is_set(ADCSRA,ADSC)); // measuring

            uint8_t low  = ADCL; // must read ADCL first - it then locks ADCH
            uint8_t high = ADCH; // unlocks both

            tmp = (high<<8) | low;
            tmp = (typVbg * 1023.0) / tmp;
            result = result + tmp;
            delay(10);
        }

  result = result / 5;
  return result;
}
