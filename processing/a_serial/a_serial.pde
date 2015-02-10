/*
 * Пример совместной работы Arduino + Processing
 * Описание http://tim4dev.com/arduino-processing-imax-b6/
 * Copyright 2015 Yuriy Tim
 */

import processing.serial.*;

// это для записи в файл
import java.io.BufferedWriter;
import java.io.FileWriter;
import java.text.SimpleDateFormat;
import java.util.Calendar;

String outFilename = "true-voltmeter.csv"; //  имя файла
PrintWriter output;

Serial arduinoPort; // serial port

// Измените эту строку, подставьте вместо "0" индекс вашего последовательного порта
// который печатается при запуске скетча
byte SerialPort = 0;

// для графика
int xPos = 0;
int lastxPos   = 0;
int lastheight = 0;
float MIN_V = 0.8;
float MAX_V = 1.7;



void setup() {
  delay(1000);
  size(600, 400);
  background(255);
  println("----------------------------------");

  // печатает все доступные serial порты
  println( Serial.list() );

  arduinoPort = new Serial(this, Serial.list()[SerialPort], 9600);
  arduinoPort.readStringUntil('\n'); // мусор пропускаем

  // для записи в файл
  output = createWriter (outFilename);
}



void draw() {
  // все интересное происходит в serialEvent
}

void serialEvent (Serial arduinoPort) {
  // получить текстовую строку из порта
  String inString = arduinoPort.readStringUntil('\n');
  if (inString != null) {
    inString = trim(inString);       // убираем пробелы

    float dataFromArduino = float( inString );  // преобразуем строку в число
    if ( !Float.isNaN( dataFromArduino ) ) {

      println(dataFromArduino); // пишем на консоль для отладки

      String date1 = myGetDateTime("dd.MM.yyyy");
      String time1 = myGetDateTime("HH:mm:ss");
      output.println( date1 +";"+ time1 +";"+ dataFromArduino ); // пишем в файл
      output.flush(); // сбрасываем на диск

      dataFromArduino = map(dataFromArduino, MIN_V, MAX_V, 0, height); //map to the screen height

      //Drawing a line from Last dataFromArduino to the new one
      stroke(192, 0, 0);  // цвет линии
      strokeWeight(2);    // толщина линии
      line(lastxPos, lastheight, xPos, height - dataFromArduino);
      lastxPos   = xPos;
      lastheight = int(height - dataFromArduino);

      // если достигнут край окна, то начать сначала
      if (xPos >= width) {
        xPos = 0;
        lastxPos = 0;
        background(0);  // очистить окно
      }
      else {
        // увеличить горизонтальную позицию
        xPos = xPos + 3;
      }
    }
  }
}



String myGetDateTime(String format1)
{
    SimpleDateFormat df = new SimpleDateFormat(format1);
    Calendar date = Calendar.getInstance();
    return(df.format(date.getTime()));
}
