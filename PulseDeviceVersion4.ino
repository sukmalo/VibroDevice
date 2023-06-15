#include "Wire.h"           //  Подключаем библиотеку для работы с шиной I2C
#include "MAX30105.h"       //  Подключаем библиотеку для работы с модулем
#include "heartRate.h"      //  Подключаем блок для работы с ЧСС (пульс)

MAX30105 PULSE_SENSOR;      //  Создаём объект для работы с библиотекой

const byte RATE_SIZE = 4;   //  Коэффициент усреднения. ЧЕм больше число, тем больше усреднение показаний.
byte rates[RATE_SIZE];      //  Массив со значениями ЧСС
byte rateSpot = 0;          //  Переменная с порядковым номером значения в массиве
long lastBeat = 0;          //  Время последнего зафиксированннного удара
float heartRateMinute;      //  ЧСС
int heartRateAVG;

#define v_pin 3             //вибромотор

void setup() {

  Serial.begin(115200);
  if(!PULSE_SENSOR.begin()){
    Serial.println("!Sensor was not found!");
    while(1);
  }
  Serial.println("Attach the sensor to the skin");
  PULSE_SENSOR.setup();
  PULSE_SENSOR.setPulseAmplitudeRed(0x0A);         //  Выключаем КРАСНЫЙ светодиод для того, чтобы модуль начал работу
  PULSE_SENSOR.setPulseAmplitudeGreen(0);          //  Выключаем ЗЕЛЁНЫЙ светодиод

  pinMode(v_pin, OUTPUT);
}



void loop() {

  long irValue = PULSE_SENSOR.getIR();            //   Считывание значение ИК-светодиода
  bool hasLastbeat = false; // declare global start

  if (checkForBeat(irValue) == true){
    //We sensed a beat!
    long delta = millis() - lastBeat;
    lastBeat = millis();

    heartRateMinute = 60 / (delta / 1000.0);

    if (heartRateMinute < 255 && heartRateMinute > 50 && hasLastbeat){
      hasLastbeat = true;
      rates[rateSpot++] = (byte)heartRateMinute; //Store this reading in the array
      rateSpot %= RATE_SIZE; //Wrap variable
      //Take average of readings
      //int heartRateAVG = 0;
      for (byte x = 0 ; x < RATE_SIZE; x++)
        heartRateAVG += rates[x];

      heartRateAVG /= RATE_SIZE;
    }

    else hasLastbeat = false;

    if (irValue > 50000){ //проверка находится ли датчик в контакте с пользователем
      Serial.print("IR = ");
      Serial.print(irValue);
      Serial.print("\t");
      Serial.print("HR NOW = ");
      Serial.print(heartRateMinute);
      Serial.print("\t");
      // Serial.print("HR AVG = ");
      // Serial.print(heartRateAVG);
      Serial.println("");

      if (heartRateMinute > 70){ //проверка на отклонение пульса от нормы
        Serial.print("Vibro is active"); //вывод сообщения что вибрации включены
        Serial.println("");

        vibroFunction();
        
        
      }
      if (heartRateMinute <= 70){ //можно было бы написать условие с else но оно так не работало
        analogWrite(v_pin, 0);
        delay(100);
      }            
    }
    else{
      Serial.print("I can't fill your finger"); //ответвление на условие если датчик находится не в контакте с пользователем
      Serial.println();
    }

  }
}

void vibroFunction(){ //алгоритм вибраций

  analogWrite(v_pin, 10); //20
  delay(300);
  analogWrite(v_pin, 15); //15
  delay(300);
  analogWrite(v_pin, 20); //20
  delay(300);
  analogWrite(v_pin, 15); //15
  delay(300);
  analogWrite(v_pin, 10); //15
  delay(1000);
}