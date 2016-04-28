#include "dht11.h"
#include <LiquidCrystal.h>

#define DHT11PIN 2
dht11 DHT11;

//LiquidCrystal lcd(12, 11, 5, 4, 3, 2);
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// Custom Character Generator for HD44780 LCD Modules
// http://omerk.github.io/lcdchargen/
byte tempChar[8] = {0b00110,0b01001,0b01001,0b00110,0b00000,0b00000,0b00000,0b00000};

int x = 0;


void setup() {
  lcd.begin(16, 2);
  lcd.clear();
  lcd.createChar(0, tempChar);
  lcd.setCursor(0,0); // col, row
  lcd.print("DHT11 + LCD Test");
  lcd.setCursor(0,1); // col, row
  lcd.print("tim4dev.com");
  delay(5000);
}

void loop() 
{
    lcd.clear();
    x++;
    int chk = DHT11.read(DHT11PIN);
 
    lcd.setCursor(0,0); // col, row
    switch (chk)
    {
        case DHTLIB_OK: 	
            break;
        case DHTLIB_ERROR_CHECKSUM: 
            lcd.print("Checksum error"); 
            delay(60000);
            return; // начать новый loop()
            break;
        case DHTLIB_ERROR_TIMEOUT: 
            lcd.print("Time out error"); 
            delay(60000);
            return; // начать новый loop()
            break;
        default: 
            lcd.print("Unknown error"); 
            delay(60000);
            return; // начать новый loop()
            break;
    }

    int i;
    for (i = 0; i < 4; i++) {
        lcd.clear();
        lcd.setCursor(0,0); // col, row
        lcd.print("Temp "); 
        lcd.print((int)DHT11.temperature); 
        lcd.write((byte)0);
        lcd.print("C  ("); lcd.print(x); lcd.print(")"); 
  
        lcd.setCursor(0,1); // col, row
        lcd.print("Humidity ");  lcd.print((int)DHT11.humidity);  lcd.print("%   ");
        
        delay(10000);
        
        lcd.clear();
        lcd.setCursor(0,0); // col, row
        lcd.print("DewPoint "); lcd.print(dewPoint(DHT11.temperature, DHT11.humidity));
        lcd.write((byte)0); lcd.print("C");
    
        lcd.setCursor(0,1); // col, row
        lcd.print("DP Fast "); lcd.print(dewPointFast(DHT11.temperature, DHT11.humidity));
        lcd.write((byte)0); lcd.print("C ");
        
        delay(10000);
    }
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

