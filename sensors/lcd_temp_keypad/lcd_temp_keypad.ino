/*
 * LCD 1602 with Keypad, DHT11
 * Copyright 2015 Yuriy Tim http://tim4dev.com
 *
 * Кнопки на клавиатуре:
 * LEFT,  UP   - показать температуру/влажность в течение 10 (DELAY_SHOW) сек
 * RIGHT, DOWN - показать точки росы в течение 10 (DELAY_SHOW) сек
 * SELECT      - перечитать информацию с датчика температуры/влажности воздуха
 */

#include "dht11.h"
#include <LiquidCrystal.h>

#define DHT11PIN 2
dht11 DHT11;

// обычно LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
// но у нас с кнопками
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// определяем кнопки
const byte bRIGHT = 0;
const byte bUP    = 1;
const byte bDOWN  = 2;
const byte bLEFT  = 3;
const byte bSELECT= 4;
const byte bNONE  = 99;
int keyIN = -1;

// различные задержки
const int DELAY_KEY = 1500;  // после надписи какая кнопка нажата
const int DELAY_SHOW= 10000; // сколько времени показывать данные
const int DELAY_FOR = 50;    // для цикла for

// Символ градуса, создан с помощью
// Custom Character Generator for HD44780 LCD Modules http://omerk.github.io/lcdchargen/
byte tempChar[8] = {0b00110,0b01001,0b01001,0b00110,0b00000,0b00000,0b00000,0b00000};

int temperature, humidity;
double dewP, dewPFast;

// Общее количество итераций/замеров
int count = 0;



/****************************************************************************
 * Главная программа
 ****************************************************************************/



// загрузка, установка
void setup() {
  lcd.begin(16, 2);
  lcd.clear();
  lcd.createChar(0, tempChar);
  lcd.setCursor(0,0); // col, row
  lcd.print("DHT11+LCD+Keypad");
  lcd.setCursor(0,1); // col, row
  lcd.print("tim4dev.com");
  delay(3000);
}

// главный цикл
void loop()
{
    int i;
    lcd.clear();

    // если ошибки при считывании датчика DHT11
    if (!readDTH11() )
        return; // начать новый loop()

    showTemperatureHumidity();

    for (i = 0; i <= DELAY_SHOW; i = i + DELAY_FOR) {
        actionKey();
        delay(50);
    }

    showDP();

    for (i = 0; i <= DELAY_SHOW; i = i + DELAY_FOR) {
        actionKey();
        delay(50);
    }
}



/****************************************************************************
 * Функции
 ****************************************************************************/



// считать информацию с датчика температуры/влажности воздуха
boolean readDTH11() {
    count++;
    int chk = DHT11.read(DHT11PIN);

    // если ошибки при считывании датчика DHT11
    if (!checkError(chk) )
        return false; // начать новый loop()

    temperature = DHT11.temperature;
    humidity    = DHT11.humidity;
    dewP        = dewPoint(temperature, humidity);
    dewPFast    = dewPointFast(temperature, humidity);

    return true;
}

// dewPoint function NOAA
// reference (1) : http://wahiduddin.net/calc/density_algorithms.htm
// reference (2) : http://www.colorado.edu/geography/weather_station/Geog_site/about.htm
//
double dewPoint(double celsius, double humidity)
{
    // (1) Saturation Vapor Pressure = ESGG(T)
    double RATIO = 373.15 / (273.15 + celsius);
    double RHS = -7.90298 * (RATIO - 1);
    RHS += 5.02808 * log10(RATIO);
    RHS += -1.3816e-7 * (pow(10, (11.344 * (1 - 1/RATIO ))) - 1) ;
    RHS += 8.1328e-3 * (pow(10, (-3.49149 * (RATIO - 1))) - 1) ;
    RHS += log10(1013.246);

    // factor -3 is to adjust units - Vapor Pressure SVP * humidity
    double VP = pow(10, RHS - 3) * humidity;

    // (2) DEWPOINT = F(Vapor Pressure)
    double T = log(VP/0.61078);   // temp var
    return (241.88 * T) / (17.558 - T);
}

// delta max = 0.6544 wrt dewPoint()
// 6.9 x faster than dewPoint()
// reference: http://en.wikipedia.org/wiki/Dew_point
double dewPointFast(double celsius, double humidity)
{
    double a = 17.271;
    double b = 237.7;
    double temp = (a * celsius) / (b + celsius) + log(humidity*0.01);
    double Td = (b * temp) / (a - temp);
    return Td;
}


boolean checkError(int chk) {
    lcd.setCursor(0,0); // col, row
    switch (chk)
    {
        case DHTLIB_OK:
            return true;
            break;
        case DHTLIB_ERROR_CHECKSUM:
            lcd.print("Checksum error ");
            delay(30000);
            return false;
            break;
        case DHTLIB_ERROR_TIMEOUT:
            lcd.print("Time out error ");
            delay(30000);
            return false;
            break;
        default:
            lcd.print("Unknown error ");
            delay(30000);
            return false;
            break;
    }
    return true;
}

// вывести на экран температуру/влажность
void showTemperatureHumidity()  {
    lcd.clear();
    lcd.setCursor(0,0); // col, row
    lcd.print("Temp ");
    lcd.print((int)temperature);
    lcd.write((byte)0);
    lcd.print("C  ("); lcd.print(count); lcd.print(")");

    lcd.setCursor(0,1); // col, row
    lcd.print("Humidity ");  lcd.print((int)humidity);  lcd.print("%   ");
}

void showDP() {
    lcd.clear();
    lcd.setCursor(0,0); // col, row
    lcd.print("DewPoint "); lcd.print(dewP);
    lcd.write((byte)0); lcd.print("C");

    lcd.setCursor(0,1); // col, row
    lcd.print("DP Fast "); lcd.print(dewPFast);
    lcd.write((byte)0); lcd.print("C ");
}


// читает какая кнопка нажата
// использован код Mark Bramwell, July 2010
int readLCDbtn()
{
  keyIN = analogRead(0); // чтение нажатой кнопки с аналогового пина 0
  // my buttons when read are centered at these valies: 0, 144, 329, 504, 741
  // we add approx 50 to those values and check to see if we are close
  if (keyIN > 1000) return bNONE; // We make this the 1st option for speed reasons since it will be the most likely result
  if (keyIN < 50)   return bRIGHT;
  if (keyIN < 195)  return bUP;
  if (keyIN < 380)  return bDOWN;
  if (keyIN < 555)  return bLEFT;
  if (keyIN < 790)  return bSELECT;
  return bNONE;
}


/*
 * Производит действия при нажатии на кнопки
 *
 * Кнопки на клавиатуре:
 * LEFT,  UP   - показать температуру/влажность в течение 10 (DELAY_SHOW) сек
 * RIGHT, DOWN - показать точки росы в течение 10 (DELAY_SHOW) сек
 * SELECT      - перечитать информацию с датчика температуры/влажности воздуха
 */
void actionKey() {
    int lcd_key;

    lcd_key = readLCDbtn();

    lcd.setCursor(0, 0); // col, row
    switch (lcd_key)
    {
        case bLEFT:
        {
            lcd.print("LEFT            ");
            delay(DELAY_KEY);
            showTemperatureHumidity();
            break;
        }
        case bUP:
        {
            lcd.print("UP              ");
            delay(DELAY_KEY);
            showTemperatureHumidity();
            break;
        }
        case bRIGHT:
        {
            lcd.print("RIGHT           ");
            delay(DELAY_KEY);
            showDP();
            break;
        }
        case bDOWN:
        {
            lcd.print("DOWN            ");
            delay(DELAY_KEY);
            showDP();
            break;
        }
        case bSELECT:
        {
            lcd.print("SELECT          ");
            delay(DELAY_KEY);
            readDTH11(); // считать информацию с датчика температуры/влажности воздуха
            showTemperatureHumidity();
            break;
        }
        case bNONE:
        {
            break;
        }
    }
}

