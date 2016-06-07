/*
 *
 * Weather Station RF-ESP-DHT-SRV
 * Код для СЕРВЕРА.
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

#define VERSION_SHORT "14/2016.04.06"
#define VERSION "Weather Station RF-ESP-DHT-SRV ver." VERSION_SHORT



// для доступа к WIFI
const String SSID      = "*****";
const String PASSWORD  = "*****";

// для доступа к веб серверу
const String DEST_HOST = "ваш хост";  // например tim4dev.com
const String DEST_PORT = "ваш порт";  // обычно 80
const String DEST_URL  = "/ваш путь/weather.php";
const String SOURCE_KEY= "ваш ключ доступа"; // должен совпадать с $access_key из config.php



/*
 * ТОЛЬКО ДЛЯ ОТЛАДКИ !
 * В ФИНАЛЬНОЙ ВЕРСИИ ЗАКОММЕНТИРОВАТЬ !
 * Закомментировать всё начинающееся с #define DEBUG*
 */
//#define DEBUG // должен быть определен, если определен хоть один из прочих DEBUG*

// для отладки nRF24L01
//#define DEBUG_RF

// для отладки ESP8266
//#define DEBUG_ESP



/**
 * Запись отладочной информации на SD карту
 *
 * Подключение SD карты к Mega используя софтверные пины
 * 5V        --- 5V
 * GND       --- GND
 * MISO (DO) --- 30
 * MOSI (DI) --- 31
 * SCK (CLK) --- 32
 * CS        --- 33
 *
 * Библиотека https://github.com/greiman/SdFat
 *
 */

/**
 * Опции настройки в файле SdFat/SdFatConfig.h
 *
 * SD_SPI_CONFIGURATION определяет SPI доступ к SD карте.
 *
 * Если SD_SPI_CONFIGUTATION = 0 , только класс SdFat определен и SdFat использует быструю кастомную реализацию SPI.
 *
 * Если SD_SPI_CONFIGURATION = 1 , только класс SdFat определен и SdFat использует стандартную либу Arduino SPI.h
 *
 * Если SD_SPI_CONFIGURATION = 2 , только класс SdFat определен и SdFat использует программные SPI пины как определено ниже.
 *
 * Если SD_SPI_CONFIGURATION = 3 , три класса определены SdFat, SdFatLibSpi, SdFatSoftSpi.
 * SdFat использует быструю кастомную реализацию SPI.
 * SdFatLibSpi использует стандартную либу Arduino.
 * SdFatSoftSpi это шаблон класса использующего Software SPI. Параметры шаблона определяют софтовые SPI пины.
 * См. пример examples/ThreeCards/ThreeCards.ino для одновременного использования всех трех классов.
 */

//#define DEBUG_LOG_SD

#include <SPI.h>
#include <SdFat.h>

#ifdef DEBUG_LOG_SD
/* Использование программного SPI
 * установить в SdFat/SdFatConfig.h
 * SD_SPI_CONFIGURATION = 3
 */
// файловая система
#if SD_SPI_CONFIGURATION >= 3
SdFatSoftSpi<30, 31, 32> sd;    // Miso, Mosi, Sck
#else
#error SD_SPI_CONFIGURATION must be set to 3 in SdFat/SdFatConfig.h
#endif

// SD chip select pin
const uint8_t chipSelect = 33;

// префикс имени лог-файла, должен быть не более 6 символов.
#define FILE_BASE_NAME "log"

// лог-файл
SdFile file;

const uint8_t BASE_NAME_SIZE = sizeof(FILE_BASE_NAME) - 1;
char fileName[13] = FILE_BASE_NAME "00.txt";

// признак ошибки связанной с записью лог-файла
byte flagErrorSD = 0;

#endif  // DEBUG_LOG_SD



#include "limits.h"

// задержка главного цикла (проверка данных от удаленных датчиков)
#ifdef DEBUG
const unsigned long DELAY_LOOP = 15 * 1000UL; // 15 сек
#else
const unsigned long DELAY_LOOP = 30 * 1000UL; // 30 сек
#endif

// Задержка главного цикла для локальных датчиков (измерения)
#ifdef DEBUG
const byte DELAY_LOCAL_SENSOR     =  20 * 1000UL/DELAY_LOOP;  // 20 сек
#else
const byte DELAY_LOCAL_SENSOR     = 600 * 1000UL/DELAY_LOOP;  // 10 мин
#endif

// счетчик циклов для локальных датчиков
unsigned int loop_count_local;


/* Для расчета сколько прошло времени (в сек) со момента снятия последних показаний датчика
 * индекс 0 - внутренний датчик, 1 (2, 3 ...) -  внешний(е)
 */
const byte COUNT_SENSOR = 2; // количество датчиков
unsigned long last_millis_sensor[COUNT_SENSOR]  = {0, 0}; // абсолютные засечки времени в мсек





/**************** nRF24L01+ ***************
 *
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



Arduino Mega          nRF24L01+
        3.3V----------VCC 2 (лучше внешнее питание)
      pin D8----------CE  3 (chip enable in)
  SS pin D53----------CSN 4 (chip select in)
 SCK pin D52----------SCK 5 (SPI clock in)
MOSI pin D51----------SDI 6 (SPI Data in)
MISO pin D50----------SDO 7 (SPI data out)
                      IRQ 8 (Interrupt output, not connected)
         GND----------GND 1 (ground in)
Инициализация: RH_NRF24(8, 53);

 */

// http://www.airspayce.com/mikem/arduino/RadioHead/classRHReliableDatagram.html
#include <RHReliableDatagram.h>
#include <RH_NRF24.h>
#include <SPI.h>

// адрес сервера
#define SERVER_ADDRESS 10
// адрес клиента
#define CLIENT_ADDRESS 11   // ИЗМЕНИТЬ для другого экземпляра !
// радиоканал
#define RF_CHANNEL  73

// установка радио драйвера
#if defined(__AVR_ATmega328P__)
RH_NRF24 radio(8, 10);
#elif defined(__AVR_ATmega1280__) || defined(__AVR_ATmega2560__)
RH_NRF24 radio(8, 53);  // MEGA
#else
RH_NRF24 radio;
#endif

// для управления надежной доставкой сообщений с помощью радио драйвера, установленного выше
RHReliableDatagram rfdata(radio, SERVER_ADDRESS);

struct DHT_DATA_STRUCTURE {
  int temperature;
  int humidity;
  float voltage; // значение питающего напряжения батареи датчика
};
DHT_DATA_STRUCTURE     dhtData;

uint8_t data[] = "OK";
// Dont put this on the stack
uint8_t  buf[RH_NRF24_MAX_MESSAGE_LEN];



/*************** ESP8266 ***************
 *
 *  Распиновка ESP8266 (смотреть сверху платы где чипы, пины внизу)
 *  GND   GPIO2   GPIO0   RX
 *  TX    CH_PD   RESET   VCC
 *
 *
 *  Подключение
 *  ESP     Arduino Mega
 *  TX  --> 10 pin (SoftwareSerial RX)
 *  RX  --> 11 pin (SoftwareSerial TX)
 *  VCC --> 3.3V
 *  GND --> GND
 *  CH_PD --> (резистор 10К) --> 3.3V Arduino
 *
 *  Опционально:
 *  GPI0 --> (резистор 10К) --> 3.3V Arduino
 *  GPI2 --> (резистор 10К) --> 3.3V Arduino
 *
 *
 * AT-команды ESP8266 для прошивки 0.22 в каталоге server/firmware/
 *
 * Изменить
 * hardware/arduino/avr/libraries/SoftwareSerial/SoftwareSerial.h
 * на
 * #define _SS_MAX_RX_BUFF 128
 *
 * hardware/arduino/avr/cores/arduino/HardwareSerial.h
 * SERIAL_TX_BUFFER_SIZE 64 --> 128
 * SERIAL_RX_BUFFER_SIZE 64 --> 128
 *
 * hardware/arduino/avr/cores/arduino/USBAPI.h
 * SERIAL_BUFFER_SIZE 64 --> 128
 *
 * Прошить прошивку
 * AT23-SDK101-nocloud.bin
 *
 * Установить скорость порта 57600 (т.к. для SoftSerial скорость порта в 115200 является большой и не гарантирует стабильную работу)
 * AT+UART_DEF=57600,8,1,0,0
 *
 */

#include <SoftwareSerial.h>
SoftwareSerial espSerial(10, 11);

/*  Для SoftSerial скорость порта в 115200 является большой и не гарантирует стабильную работу.
 *  Появляется «мусор» в выводе команд.
 */
#define BAUD_SERIAL 57600
/*
 * С ESP8266 нужно предварительно соединиться на 115200, задать скорость порта 57600 и сохранить настройки
 * во flash для загрузки при следующем старте модуля:
 * AT+UART_DEF=57600,8,1,0,0
 */
#define BAUD_ESP    57600

// Буфер для обмена данными с ESP8266
#define ESP_BUF_LEN  256
char    espBuf[ESP_BUF_LEN];
unsigned int espBufPos;    // указатель на конец данных в буфере
int espState;

uint8_t from_sensor; // адрес удаленного датчика


typedef enum esp_response {
    ESP_BUSY     = -5,
    ESP_ERROR    = -4,
    ESP_OVERFLOW = -3,  //  внутренний буфер переполнен
    ESP_UNKNOWN  = -2,
    ESP_TIMEOUT  = -1,
    ESP_SUCCESS  = 0
};

// счётчик числа попыток подключения к WiFi
byte counterBadWiFi = 0;

// признак ошибки передачи данных на веб-сервер
byte flagErrorWeb = 0;

// признак ошибки связанной с модулем ESP8266
byte flagErrorEsp = 0;


/* Подключение датчика температуры и влажности DHT11 (SainSmart)

    DHT11                 Arduino Mega
    лицевая сторона
    слева направо
    DATA ----------------- D2     (см. ниже DHTPIN)
    VCC  ----------------- 3.3-5В (рекомендуется 5В, лучше внешнее питание)
    GND  ----------------- GND

    Библиотека https://github.com/adafruit/DHT-sensor-library
 */
#include "DHT.h"
#define DHTPIN  2      // цифровой пин
#define DHTTYPE DHT11  // см. DHT.h

// инициализация
DHT dht(DHTPIN, DHTTYPE);





/*
 * Подключение датчика атмосферного давления BMP180 (барометр) + температура
 * по интерфейсу  I2C/TWI
 *
 * BMP180           Arduino Mega
 * VCC не подключен
 * GND ------------- GND
 * SCL ------------- 21 (SCL)
 * SDA ------------- 20 (SDA)
 * 3.3 ------------- 3.3V
 *
 * Для UNO : A4 (SDA), A5 (SCL)
 *
 * Библиотека https://github.com/adafruit/Adafruit_BMP085_Unified
 * Требует наличие библиотеки https://github.com/adafruit/Adafruit_Sensor
 */
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BMP085_U.h>
// инициализация
Adafruit_BMP085_Unified bmp = Adafruit_BMP085_Unified(10085); // sensorID

sensors_event_t event;
float pressure_bmp;
float temperature_bmp;

// признак ошибки связанной с локальным датчиком
byte flagErrorSensor = 0;





/*************** Дисплей LCD1604 ***************
 *
 *
 */
#include <LiquidCrystal.h>

LiquidCrystal lcd(22, 23, 24, 25, 26, 27);

#define LCD_MAX_ROWS 4
#define LCD_MAX_COLS 16
char lcd_pre_buf[LCD_MAX_COLS * 2];
char lcd_buf[LCD_MAX_COLS];

/* http://omerk.github.io/lcdchargen/
 * иконка идущего человека  - типа «погода за дверью»
 */
const byte idCharOutdoor = 0;
byte charOutdoor[8] = {
  0b00110,
  0b00110,
  0b00100,
  0b11110,
  0b10101,
  0b00100,
  0b01010,
  0b10010
};
// иконка типа «погода дома»
const byte idCharHome = 1;
byte charHome[8] = {
  0b00000,
  0b00000,
  0b00100,
  0b01010,
  0b10001,
  0b01110,
  0b01010,
  0b01110
};



/*
 * Вольтметр.
 * Для измерения питающего напряжения батареи
 * подробнее здесь http://tim4dev.com/arduino-secret-true-voltmeter/
 */
//#define V_PIN   1

// количество измерений
#define COUNT_MEASURE 5
// резисторы делителя напряжения
//const float r1 = 100400; // 100K
//const float r2 =  9960;  // 10K
// эту константу необходимо откалибровать индивидуально
// как описано здесь http://tim4dev.com/arduino-secret-true-voltmeter/
//const float typVbg = 1.082; // обычно в пределах 1.0 -- 1.2 В






/****************************************************************************************
 *  MAIN program
 ****************************************************************************************/

void setup()
{
    Serial.begin(BAUD_SERIAL);
    #ifdef DEBUG
    Serial.println("\n\n\n\n\n\n\n******* " + VERSION + " *******\n\n");
    Serial.println("-------");
    Serial.print("SERIAL_RX_BUFFER_SIZE = "); Serial.println(SERIAL_RX_BUFFER_SIZE);
    Serial.print("SERIAL_TX_BUFFER_SIZE = "); Serial.println(SERIAL_TX_BUFFER_SIZE);
    Serial.print("_SS_MAX_RX_BUFF = ");       Serial.println(_SS_MAX_RX_BUFF);
    Serial.println("-------");
    #endif

    //******* LCD 1604 *******
    lcd.createChar(idCharOutdoor, charOutdoor);
    lcd.createChar(idCharHome,    charHome);
    lcd.begin(LCD_MAX_COLS, LCD_MAX_ROWS);
    lcd.clear();
    lcdPrintInfo(VERSION_SHORT);

    delay(3000);

    #ifdef DEBUG_LOG_SD
    // запись в лог-файл на SD карту
    if ( sd.begin(chipSelect, SPI_HALF_SPEED) ) {
        lcdPrintInfo("SD init OK");
        // определяем имя лог-файла и создаем его
        logLogCreate();
        logSeparator();
        logData("***** Setup begin. " + String(VERSION) + " *****");
        delay(1000);
    } else {
        lcdPrintInfo("SD init ERROR");
        delay(30000);
    }
    #endif


    //******* настройка радио *******
    myError( "RF init",    rfdata.init() ); // по умолчанию устанавливается 2.402 GHz (канал 2), 2Mbps, 0dBm
    myError( "setChannel", radio.setChannel(RF_CHANNEL) );  // устанавливаем свой канал
    myError( "setRF",      radio.setRF(RH_NRF24::DataRate1Mbps, RH_NRF24::TransmitPower0dBm) );  // устанавливаем скорость 1Mbps
    rfdata.setTimeout(400);  // время таймаута передачи, по умолчанию 200мс

    //******* настройка WiFi *******
    espSerial.begin(BAUD_ESP);   // соединение с ESP8266
    delay(1000);

    // ESP8266 отвечает на команду AT ?
    espIsAlive();

    // переключение в режим Station (WiFi клиент)
    espSendCmd("AT+CWMODE_CUR=1", "OK", 3000);

    // подключение к точке доступа
    espState = espConnectToWiFi();
    if ( espState != ESP_SUCCESS )  {
        delay(5000);
        Serial.println("WiFi not connected! Try again ...");
        espConnectToWiFi();
    }

    /*
     * Выбрать режим одиночного подключения TCP/IP
     * AT+CIPMUX=0
     * Возвращает : OK
     */
    espSendCmd("AT+CIPMUX=0", "OK", 2000);

    //******* DHT11 *******
    #ifdef DEBUG
    Serial.println("DHT11.begin...");
    #endif

    dht.begin();

    //******* BMP180 *******
    myError( "BMP180.begin", bmp.begin() );

    #ifdef DEBUG
    sensor_t sensor;
    bmp.getSensor(&sensor);
    Serial.println("------------------------------------");
    Serial.print  ("Sensor:       "); Serial.println(sensor.name);
    Serial.print  ("Driver Ver:   "); Serial.println(sensor.version);
    Serial.print  ("Unique ID:    "); Serial.println(sensor.sensor_id);
    Serial.print  ("Max Value:    "); Serial.print(sensor.max_value); Serial.println(" hPa");
    Serial.print  ("Min Value:    "); Serial.print(sensor.min_value); Serial.println(" hPa");
    Serial.print  ("Resolution:   "); Serial.print(sensor.resolution); Serial.println(" hPa");
    Serial.println("------------------------------------");
    #endif

    loop_count_local = 0;  // сброс счетчика

    #ifdef DEBUG
    Serial.println("\n\n******* Setup complete *******\n\n\n\n");
    #endif

    lcdPrintInfo("Setup complete");

    #ifdef DEBUG_LOG_SD
    logData("***** Setup complete *****");
    logSeparator();
    #endif
}



void loop()
{
    #ifdef DEBUG
    Serial.println("loop start");
    Serial.print("DELAY_LOCAL_SENSOR = "); Serial.println(DELAY_LOCAL_SENSOR);
    Serial.print("loop_count_local = ");         Serial.println(loop_count_local);
    #endif

    // пришло ли что-нибудь новое от заоконных датчиков?
    if ( rfdata.available() )    {
        #ifdef DEBUG_RF
        Serial.println("RF data available");
        #endif

        #ifdef DEBUG_LOG_SD
        logSeparator();
        logData("START. Remote data available.");
        #endif
        uint8_t buflen = sizeof(buf);
        uint8_t to;
        // ждем сообщение адресованное нам от клиента
        // необходимо вызывать эту фунцию достаточно часто, чтобы не пропустить входящее сообщение
        if ( rfdata.recvfromAck(buf, &buflen, &from_sensor, &to) )  {

            // нам ли предназначены эти данные ?
            if ( SERVER_ADDRESS == to ) {

                memcpy(&dhtData, buf, sizeof(dhtData));

                #ifdef DEBUG
                Serial.println("-------");
                Serial.print("From address: "); Serial.println(from_sensor);
                Serial.print("Temperature :\t"); Serial.print(dhtData.temperature); Serial.print(" C\t");
                Serial.print("Humidity : ");     Serial.print(dhtData.humidity);    Serial.print(" %\t");
                Serial.print("Voltage : ");      Serial.print(dhtData.voltage);     Serial.println(" V");
                #endif

                #ifdef DEBUG_LOG_SD
                logData("From sensor : " + String(from_sensor, DEC) );
                logData("Temperature : " + String(dhtData.temperature, DEC) );
                logData("Humidity    : " + String(dhtData.humidity, DEC) );
                #endif

                // посылаем ответ клиенту (с повторами, если потребуется)
                // по умолчанию: 3 повтора, с таймаутами (200мс) по retries*timeout
                // можно изменить с помощью setRetries() и setTimeout()
                myError( "sendtoWait", rfdata.sendtoWait(data, sizeof(data), from_sensor) );

                // отсылаем данные на вебсервер, тип данных type=dht
                espSendData( "type=dht&t="+ String(dhtData.temperature) +"&h="+ String(dhtData.humidity) +"&v="+ String(dhtData.voltage) + "&s="+ String(from_sensor) );

                // void lcdPrintOutdoor(int temperature, int humidity, float voltage)
                lcdPrintOutdoor(dhtData.temperature, dhtData.humidity, dhtData.voltage);

                // для расчета сколько прошло времени (в сек) со момента снятия последних показаний датчика
                // индекс 0 - внутренний датчик, 1 (2, 3 ...) -  внешний(е)
                last_millis_sensor[1] = millis();
            }
            else {
                #ifdef DEBUG
                Serial.println("UNKNOWN «TO» address !");
                #endif
            }
        }

        #ifdef DEBUG_LOG_SD
        logData("END. Remote data.");
        logSeparator();
        #endif
    }

    /*
     * Считываем показания локального датчика температуры и влажности DHT11
     */
    if ( DELAY_LOCAL_SENSOR < loop_count_local ) {
        #ifdef DEBUG_LOG_SD
        logSeparator();
        logData("START. Read local sensor.");
        #endif

        loop_count_local = 0;  // сброс счетчика

        // делаем несколько замеров температуры и влажности
        // и находим среднее арифметическое
        myDhtRead();

        dhtData.voltage     = 5.01; // debug !
        //dhtData.voltage     = getVoltage();
        delay(200); // задержка перед повторным считыванием данных

        #ifdef DEBUG
        Serial.println("-------");
        Serial.println("From LOCAL sensors");
        Serial.print("Temperature (DHT11):\t"); Serial.print(dhtData.temperature); Serial.print(" C\t");
        Serial.print("Humidity    (DHT11): ");  Serial.print(dhtData.humidity);    Serial.print(" %\t");
        Serial.print("Voltage: ");  Serial.print(dhtData.voltage);     Serial.println(" V");
        #endif

        #ifdef DEBUG_LOG_SD
        logData("Temperature : " + String(dhtData.temperature, DEC) );
        logData("Humidity    : " + String(dhtData.humidity, DEC) );
        #endif

        // отсылаем данные на вебсервер, тип данных type=dht
        espSendData( "type=dht&t="+ String(dhtData.temperature) +"&h="+ String(dhtData.humidity) +"&v="+ String(dhtData.voltage) + "&s="+ String(CLIENT_ADDRESS) );

        /*
         * Считываем показания локального барометра BMP180
         */
        // Get a new sensor event
        bmp.getEvent(&event);

        if (event.pressure)  {
            flagErrorSensor = 0;
            // Давление в гПа (на уровне станции)
            // 100 Па = гектопаскаль гПа hPa
            // 1 Па = 0,0075 мм рт. ст. или 1 мм рт. ст. = 133,3 Па
            pressure_bmp = event.pressure;
            bmp.getTemperature(&temperature_bmp);
            temperature_bmp = round(temperature_bmp);

            #ifdef DEBUG
            Serial.print("Pressure (BMP180):\t");   Serial.print( pressure_bmp );    Serial.print(" hPa (standart = 1013 hPa)\t");
            Serial.print("Temperature (BMP180): "); Serial.print( temperature_bmp ); Serial.println(" C");
            #endif

            #ifdef DEBUG_LOG_SD
            logData("Pressure : " + String(pressure_bmp, DEC) );
            #endif

            // отсылаем данные на вебсервер, тип данных type=bmp
            espSendData( "type=bmp&t="+ String(temperature_bmp) +"&p="+ String(pressure_bmp) + "&s="+ String(CLIENT_ADDRESS) );

            // void lcdPrintHome(int temperature, int humidity, int pressure)
            lcdPrintHome(dhtData.temperature, dhtData.humidity, pressure_bmp);

            delay(200); // минимальная задержка перед повторным считыванием данных
        }
        else  {
            flagErrorSensor++;
            myError( "BMP180 get data", false );
        }
        // для расчета сколько прошло времени (в сек) со момента снятия последних показаний датчика
        // индекс 0 - внутренний датчик, 1 (2, 3 ...) -  внешний(е)
        last_millis_sensor[0] = millis();

        #ifdef DEBUG_LOG_SD
        logData("END. Local sensor data.");
        logSeparator();
        #endif
    }

    loop_count_local++;
    delay(DELAY_LOOP);

    // печатаем сколько прошло времени (в сек) со момента снятия последних показаний датчиков
    lcdPrintLastSensorTime();
}



/****************************************************************************
 * Функции
 ****************************************************************************/



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



/*************** DHT ***************/

// делаем несколько замеров температуры и влажности
// и находим среднее арифметическое
void myDhtRead()
{
    float curTemp = 0.0;
    float curHum  = 0.0;
    for (int i = 0; i < COUNT_MEASURE; i++) {
        curTemp = curTemp + dht.readTemperature();
        curHum  = curTemp + dht.readHumidity();
        delay(50);
    }
    dhtData.temperature = round( curTemp / COUNT_MEASURE );
    dhtData.humidity    = round( curHum  / COUNT_MEASURE );
}



/*************** ESP8266 ***************/

/*
 * На входе:
 * cmd  - посылаемая команда
 * test - ожидаемый ответ
 * timeout - таймаут
 */
int espSendCmd(const String& cmd, const String& test, unsigned int timeout) {
    delay(100);

    #ifdef DEBUG_ESP
    Serial.print("\nsend command --> ");
    Serial.println(cmd);
    #endif

    #ifdef DEBUG_LOG_SD
    logData("START. espSendCmd()");
    logData(cmd);
    #endif

    // посылаем команду в ESP
    espSerial.print(cmd);
    espSerial.print("\r\n");
    delay(500);

    // заполняем буфер ответа и читаем вывод
    espState = espReadResponse(test.c_str(), timeout);
    #ifdef DEBUG_ESP
    printEspBuf();  // печатаем содержимое буфера
    #endif

    #ifdef DEBUG_LOG_SD
    logEspBuf();  // пишем буфер в лог-файл
    logData("END. espSendCmd()");
    #endif

    return espState;
}

/*
 * Чтение вывода от ESP8266.
 *
 * Вход :
 * test - искомая строка
 * timeout - таймаут
 *
 * Возвращает:
 * ESP_SUCCESS - если весь вывод считан в буфер
 * ESP_TIMEOUT - выход по таймауту, никаких данных не получено
 * ESP_OVERFLOW- буфер переполнен
 * ESP_ERROR   - обнаружена ошибка
 *
 */
int espReadResponse(const char * test, unsigned int timeout)
{
    unsigned long timestamp1 = millis();    // Timestamp coming into function
    int i = 0;

    // очищаем буфер
    memset(espBuf, '\0', ESP_BUF_LEN);
    espBufPos = 0;

    // пока не заполнится буфер или не таймаут
    while ( timestamp1 + timeout > millis() )  {
        while ( espSerial.available() > 0 )  {

            if ( espSearchBuffer(test) )    {
                espShowResponse(ESP_SUCCESS);
                return ESP_SUCCESS;
            }

            if ( timestamp1 + timeout > millis() ) {

                // сохраняем данные в буфере
                espBuf[espBufPos] = espSerial.read();

                if ( espBufPos + 1 <= ESP_BUF_LEN )  {
                    espBufPos++;
                } else {
                    // буфер переполнен, зацикливаем
                    espBufPos = 0;
                }
            } else {
                // таймаут
                espShowResponse(ESP_TIMEOUT);
                return ESP_TIMEOUT;
            }
        } // while espSerial

        if ( 0 == espSerial.available() )   {
            // ~174 миллисекунды ждем следующий байт от ESP (по мотивам SparkFun)
            delay( (1 / BAUD_ESP) * 10000000UL );
        }
    } // while millis

    if ( espSearchBuffer(test) )    {
        espShowResponse(ESP_SUCCESS);
        return ESP_SUCCESS;
    }
    if ( espSearchBuffer("busy") )  {
        espShowResponse(ESP_BUSY);
        return ESP_BUSY;
    }
    if ( espSearchBuffer("ERROR") ) {
        espShowResponse(ESP_ERROR);
        return ESP_ERROR;
    }
    espShowResponse(ESP_UNKNOWN);
    return ESP_UNKNOWN;
}

/*
 * Поиск строки в буфере
 * Вход: test - искомая строка
 * Возвращает:
 * true - строка найдена
 * false - в противном случае
 */
boolean espSearchBuffer(const char * test)
{
    if ( strstr( (const char *)espBuf, test) != NULL )  // нашли искомую строку
      return true;
    else
      return false;
}

void printEspBuf()
{
    #ifdef DEBUG_ESP
    Serial.print("--- ESP buffer ["); Serial.print(espBufPos); Serial.print("]"); Serial.println(" ---");
    for (int i = 0; i <= espBufPos; i++)  {
        Serial.print( (char)espBuf[i] );
    }
    Serial.println("----------------------");
    #endif
}

void espShowResponse(int espState)  {
    String msg;

    switch (espState)  {
      case ESP_SUCCESS:
        msg = "ESP_SUCCESS";
        break;
      case ESP_UNKNOWN:
        msg = "ESP_UNKNOWN";
        break;
      case ESP_TIMEOUT:
        msg = "ESP_TIMEOUT";
        break;
      case ESP_OVERFLOW:
        msg = "ESP_OVERFLOW";
        break;
      case ESP_ERROR:
        msg = "ESP_ERROR";
        break;
      case ESP_BUSY:
        msg = "ESP_BUSY";
        break;
      default:
        msg = "ESP unknown state code";
        break;
    }

    #ifdef DEBUG_ESP
    Serial.println(msg);
    #endif

    #ifdef DEBUG_LOG_SD
    logData(msg);
    #endif
}

/*
 * Очищаем буфер при его переполнении - просто читаем весь вывод
 */
/*void espFlush() {
    while ( espSerial.available() > 0 ) {
        espSerial.read();
    }
}*/

void espSendData(const String& uri) {
    #ifdef DEBUG_ESP
    Serial.println("\nespSendData()");
    #endif

    #ifdef DEBUG_LOG_SD
    logData("START. espSendData()");
    logData(uri);
    #endif

    // переподключение к точке доступа (если нужно)
    if ( !espIsWiFiConnected() )
        espConnectToWiFi();

    if ( !espIsAlive() )
        return;

    espSendCmd("AT+CIPSTART=\"TCP\",\""+ DEST_HOST + "\"," + DEST_PORT, "OK", 7000);
    delay(200);

    /*
     * Отправить данные TCP/IP
     * AT+CIPSEND=<длина>
     * Возвращает
     * Recv 98 bytes
     * SEND OK
     * CLOSED
     * или
     * ERROR
     *
     * Ответ сервера начинается с
     * +IPD,<длина>:...
     */
    String cmd = "GET "+ DEST_URL +"?" + uri +
        "&m=" + String(millis()) +
        "&k=" + SOURCE_KEY +
        " HTTP/1.1\r\nHost: "+ DEST_HOST +"\r\n\r\n";

    // ждать символ консоли ">"
    espSendCmd("AT+CIPSEND=" + String(cmd.length()), ">", 5000);
    delay(200);

    // послать остальные данные
    if ( espSendCmd(cmd, "SEND OK", 7000) != ESP_SUCCESS ) {
        // ищем другие строки, которые бы указывали на успех
        if ( (espSearchBuffer("+IPD,") != ESP_SUCCESS) && (espSearchBuffer("SEND") != ESP_SUCCESS) )  {
            flagErrorWeb = 1;
            espShowResponse(ESP_ERROR);
        } else {
            flagErrorWeb = 0;
            espShowResponse(ESP_SUCCESS);
        }
    }

    /*
     * Закрыть соединение TCP
     * AT+CIPCLOSE
     */
    espSendCmd("AT+CIPCLOSE", "OK", 2000);

    #ifdef DEBUG_LOG_SD
    logData("END. espSendData()");
    #endif
}



boolean espIsAlive() {
    boolean state1 = false;
    byte count = 3; // кол-во повторов
    byte     i = 0;
    String   msg;

    while ( !state1 && (i < count) )    {
        state1 = ( ESP_SUCCESS == espSendCmd("AT", "OK", 2000) );
        i++;
        if ( !state1)
            delay(5000);
    }
    if ( state1 )  {
        flagErrorEsp = 0;
    } else {
        flagErrorEsp = 1;
    }
    lcdPrintStatus();

    if (state1)
        msg = "ESP8266 is alive OK";
    else
        msg = "ESP8266 not alive ERROR!";

    #ifdef DEBUG_ESP
    Serial.println(msg);
    #endif

    #ifdef DEBUG_LOG_SD
    logData("espIsAlive() : " + msg);
    #endif

    return state1;
}



/* подключение к точке доступа
     * AT+CWJAP_CUR="SSID","PASSWORD"
     * Возвращает несколько строк:
     * WIFI CONNECTED
     * WIFI GOT IP
     * OK
     *
     * или FAIL
     */
int espConnectToWiFi()    {
    return espSendCmd("AT+CWJAP_CUR=\"" + SSID + "\",\"" + PASSWORD + "\"", "OK", 10000);
}

/*
 * Проверка подключения ESP8266 к WiFi
 * AT+CWJAP_CUR?
 * Возвращает несколько строк:
 * +CWJAP_CUR:"SSID","<mac address>"
 * OK
 * или:
 * No AP
 * OK
 */
boolean espIsWiFiConnected() {
    if ( ESP_SUCCESS == espSendCmd("AT+CWJAP_CUR?", SSID, 10000) )   {
        counterBadWiFi = 0;
        lcdPrintStatus();
        return true;
    } else {
        ++counterBadWiFi;
        lcdPrintStatus();
        if ( counterBadWiFi >= 3 )    {
            #ifdef DEBUG_LOG_SD
            logData("espIsWiFiConnected() : ESP restart");
            #endif
            espSendCmd("AT+RST", "OK", 10000); // Перезапуск модуля
            delay(1000);
            espConnectToWiFi(); // переподключение к WiFi
        }
        return false;
    }
}



/*
 * Для отображения на дисплее LCD 1604
 */

void lcdClearRow(int row)
{
    if ( row > (LCD_MAX_ROWS - 1) )
        row = LCD_MAX_ROWS - 1;
    if ( row < 0 )
        row = 0;
    // очищаем строку
    lcd.setCursor(0, row); // колонка, строка
    memset(lcd_buf, ' ', LCD_MAX_COLS);
    lcd.print(lcd_buf);
    // устанавливаем курсор в начало строки
    lcd.setCursor(0, row); // колонка, строка
}

// Печатает на экране показания удалённых, уличных датчиков
void lcdPrintOutdoor(int temperature, int humidity, float voltage)
{
    char vv[5];

    lcdClearRow(0);
    lcd.write((uint8_t)idCharOutdoor);

    dtostrf(voltage, 3, 1, vv);  // dtostrf(floatVar, minStringWidthIncDecimalPoint, numVarsAfterDecimal, charBuf);
    sprintf ( lcd_pre_buf, " %3i%c %2i%% %sv", temperature, (char)0b11011111, humidity, vv );
    // обрезка слишком длинной строки http://stackoverflow.com/questions/2114377/strings-in-c-how-to-get-substring
    strncpy(lcd_buf, lcd_pre_buf, LCD_MAX_COLS-1);
    lcd_buf[LCD_MAX_COLS] = '\0';

    lcd.print(lcd_buf);
    #ifdef DEBUG
    Serial.print("lcdPrintOutdoor() : "); Serial.println(lcd_buf);
    #endif

    #ifdef DEBUG_LOG_SD
    logData("lcdPrintOutdoor() : " + String(lcd_buf));
    #endif
}

// Печатает на экране показания внутренних, домашних датчиков
void lcdPrintHome(int temperature, int humidity, int pressure)
{
    lcdClearRow(1);
    lcd.write((uint8_t)idCharHome);

    sprintf ( lcd_pre_buf, " %3i%c %2i%% %4i", temperature, (char)0b11011111, humidity, pressure );
    // обрезка слишком длинной строки http://stackoverflow.com/questions/2114377/strings-in-c-how-to-get-substring
    strncpy(lcd_buf, lcd_pre_buf, LCD_MAX_COLS-1);
    lcd_buf[LCD_MAX_COLS] = '\0';

    lcd.print(lcd_buf);
    #ifdef DEBUG
    Serial.print("lcdPrintHome() : "); Serial.println(lcd_buf);
    #endif

    #ifdef DEBUG_LOG_SD
    logData("lcdPrintHome() : " + String(lcd_buf));
    #endif
}

// Печатает информацию в последней (нижней) строке экрана
void lcdPrintInfo(char info[LCD_MAX_COLS])
{
    // очищаем информационную строку
    lcdClearRow(3);
    // выводим информацию
    lcd.print(info);
    #ifdef DEBUG
    Serial.print("LCD : "); Serial.println(info);
    #endif
}

// Печатаем статусную информацию в последней (нижней) строке экрана
void lcdPrintStatus()
{
    /*
     * Формируем строку, где
     * s - локальный датчик давления, переменная flagErrorSensor
     * e - ESP8266, переменная flagErrorEsp
     * i - WiFi, переменная counterBadWiFi
     * w - web сервер, переменная  flagErrorWeb
     * l - лог-файл на SD карте, flagErrorSD
     * «плюс» или «минус» - нет/есть ошибки, проблемы
     */
    String str = "";

    if ( flagErrorSensor <= 0 )
        str += "s+";
    else
        str += "s-";

    if ( flagErrorEsp <= 0 )
        str += "e+";
    else
        str += "e-";

    if ( counterBadWiFi <= 0 )
        str += "i+";
    else
        str += "i-";

    if ( flagErrorWeb <= 0 )
        str += "w+";
    else
        str += "w-";

    #ifdef DEBUG_LOG_SD
    if ( flagErrorSD <= 0 )
        str += "l+";
    else
        str += "l-";
    logData( "lcdPrintStatus() : " + str);
    #endif

    // очищаем информационную строку
    lcdClearRow(3);
    // выводим информацию
    lcd.print(str);
    #ifdef DEBUG
    Serial.print("lcdPrintStatus() : "); Serial.println(str);
    #endif
}

/*
 * Печатаем (в 3-ей строке экрана) сколько прошло времени (в сек) со момента снятия последних показаний датчика
 */
void lcdPrintLastSensorTime() {
    unsigned long last_seconds_sensor = 0;

    // очищаем строку
    lcdClearRow(2);
    lcd.print("Last ");

    for (byte i = 0; i < COUNT_SENSOR; i++) {
        // сколько прошло времени (в сек) со момента снятия последних показаний датчика
        if ( millis() > last_millis_sensor[i] )  {
            last_seconds_sensor = ( millis() - last_millis_sensor[i] ) / 1000UL; // в сек
        } else {
            // произошло обнуление millis()
            last_seconds_sensor = ( ULONG_MAX - last_millis_sensor[i] + millis() ) / 1000UL;  // в сек
        }
        // выводим информацию
        lcd.print(last_seconds_sensor);
        lcd.print(",");
    } // for
}



/*
 * Запись отладочной информации на SD карту, см #define DEBUG_SD
 *
 */

#ifdef DEBUG_LOG_SD

void logData(const String& msg) {
    // пишем в лог-файл, вместе с временными метками (секунды) с момента начала выполнения скетча.
    // Это время сбрасывается на ноль, в следствие переполнения значения, приблизительно через 50 дней
    if ( !file.open(fileName, O_APPEND | O_WRITE) ) {
        lcdPrintInfo("SD open ERROR");
        flagErrorSD++;
        return;
    }
    file.print( millis() );
    file.print("\t");
    file.print(msg);
    file.println();
    file.close();
}

void logEspBuf()
{
    // пишем в лог-файл, вместе с временными метками (секунды) с момента начала выполнения скетча.
    // Это время сбрасывается на ноль, в следствие переполнения значения, приблизительно через 50 дней
    if ( !file.open(fileName, O_APPEND | O_WRITE) ) {
        lcdPrintInfo("SD open ERROR");
        flagErrorSD++;
        return;
    }
    file.print( millis() );
    file.print("\t--- ESP buffer ["); file.print(espBufPos, DEC); file.print("]"); file.println(" ---");
    for (int i = 0; i <= espBufPos; i++)  {

        //file.print( (char)espBuf[i] ); // пишем все символы

        // непечатные символы записываем точкой
        if (espBuf[i] > 31)
            file.print( (char)espBuf[i] );
        else
            file.print( '.' );

    }
    file.println("\n----------------------");
    file.close();
}


// разделитель для удобства чтения лога
void logSeparator() {
    logData("\n\n\n*********************\n\n\n");
}


// ищем новое имя для записи лог-файла и создаем его
void logLogCreate()    {
    flagErrorSD = 0;

    if (BASE_NAME_SIZE > 6) {
        lcdPrintInfo("SD name ERROR");
        flagErrorSD++;
        delay(3000);
        return;
    }
    while (sd.exists(fileName)) {
        if (fileName[BASE_NAME_SIZE + 1] != '9') {
            fileName[BASE_NAME_SIZE + 1]++;
        } else if (fileName[BASE_NAME_SIZE] != '9') {
            fileName[BASE_NAME_SIZE + 1] = '0';
            fileName[BASE_NAME_SIZE]++;
        } else {
            lcdPrintInfo("SD create ERROR");
            flagErrorSD++;
            delay(3000);
            return;
        }
    } // while
    // создаем файл
    if ( !file.open(fileName, O_CREAT | O_WRITE | O_EXCL) ) {
        lcdPrintInfo("SD file ERROR");
        flagErrorSD++;
        delay(1000);
        return;
    }
    file.close();

    lcdPrintInfo("SD create OK");
    delay(2000);
}

// DEBUG_LOG_SD
#endif
