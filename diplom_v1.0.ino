#include <MsTimer2.h>
//#include <GyverTimers.h>
#include <EEPROM.h>
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C lcd(0x27,20,4);// встановлюємо адресу LCD в 0x20 для дисплею з 20 символами в 4 рядках
#include <OneWire.h>  //библиотека для работы с датчиком температуры
#include <DallasTemperature.h>  //библиотека для работы с датчиком температуры

#define ONE_WIRE_BUS 4  //пин для снятия показаний с датчика температуры
#define water_power 5  //питание датчика затопления
#define sensor1_power 6  //питание датчика температуры
#define sensor2_power 7  //питание датчика движения коленвала
#define sensor3_power 8  //питание датчика температуры
#define analogPin A3  //пин для снятия показаний с датчика затапления
#define rel_1 11
#define rel_2 12
#define rel_vent 13
int p=0;
int t; //счетчик циклов для температуры
int h; //счетчик циклов для уровня топлива
int val = 0; // переменная для хранения считываемого значения
int val_1 = 0;
float pi = 3.14;
unsigned long lastturnkol; //переменные хранения времени оборотов коленвала
unsigned long lastturnkol_1; //переменные хранения времени оборотов коленвала
unsigned long lastturn; //переменные хранения времени
float SPEED; //переменная хранения скорости в виде десятичной дроби
float SPEED_w;
float DIST; //переменная хранения расстояния в виде десятичной дроби
float DIST_1; //переменная хранения расстояния в виде десятичной дроби
float w_length = 4; //длина окружности колеса в метрах
int RPM;
int RPM_1;
float kor_ygol = 0;
float ktime;
float period;
float ygol_dat = 45;
float t_ygol_dat;
bool vent;

//--Переменные для вывода на индикатор--
int cel_sp;
int sot_sp;
int des_sp;
int ed_sp;
int cel_di;
int dectus_di;
int tus_di;
int sot_di;
int des_di;
int ed_di;
int dr_di;
int cel_di_1;
int sot_di_1;
int des_di_1;
int ed_di_1;
int dr_di_1;
float cel_t;
int sot_t;
int des_t;
int ed_t;
int sot_val;
int des_val;
int ed_val;
int tus_ob;
int sot_ob;
int des_ob;
int ed_ob;
//--Переменные для вывода на индикатор--

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);

void setup() {
  Serial.begin(9600);  //открыть порт
  //---дисплей----
  lcd.init();                      // initialize the lcd 
  lcd.backlight();;
  byte bukva_grad[8] = {
  B00111,
  B00101,
  B00111,
  B00000,
  B00000,
  B00000,
  B00000
  };
  lcd.createChar(0, bukva_grad);
  //---дисплей---

  attachInterrupt(1,senskol,RISING); //подключить прерывание на 3 пин при повышении сигнала
  attachInterrupt(0,sens,RISING); //подключить прерывание на 2 пин при повышении сигнала
  DIST=(float)EEPROM.read(0)/10.0; //вспоминаем пройденное расстояние при запуске системы (деление на 10 нужно для сохранения десятых долей расстояния, см. запись)
  pinMode(analogPin,INPUT);
  pinMode(water_power, OUTPUT);   //пин water_power как выход
  pinMode(sensor1_power, OUTPUT); //пин sensor1_power как выход
  pinMode(sensor2_power, OUTPUT); //пин sensor2_power как выход
  pinMode(sensor3_power, OUTPUT); //пин sensor3_power как выход
  pinMode(9, OUTPUT);
  pinMode(rel_1, OUTPUT);
  pinMode(rel_2, OUTPUT);
  pinMode(rel_vent, OUTPUT);
  digitalWrite(sensor1_power, HIGH); //подать 5 вольт на пин sensor1_power
  digitalWrite(sensor2_power, HIGH);  //подать 5 вольт на пин sensor2_power
  digitalWrite(sensor3_power, HIGH);  //подать 5 вольт на пин sensor3_power
  digitalWrite(9, HIGH);
  digitalWrite(rel_1, HIGH);
  digitalWrite(rel_2, LOW);
  digitalWrite(rel_vent, LOW);
  sensors.begin();
}

void flash(){
      digitalWrite(rel_1,! digitalRead(rel_1));
      digitalWrite(rel_2,! digitalRead(rel_2));
      MsTimer2::stop();
      p++;
      //Serial.print("искра ");Serial.println(p);
}

void senskol() {
    RPM=60/((float)(micros()-lastturnkol)/1000000);  //расчет
    lastturnkol_1 = micros()-lastturnkol;
    lastturnkol = micros();  //запомнить время последнего оборота
    MsTimer2::stop();
    MsTimer2::set(period, flash);
    MsTimer2::start();
    
}

void sens() {
  if (millis()-lastturn > 80) {  //защита от случайных измерений (основано на том, что транспорт не будет ехать быстрее 120 кмч)
    SPEED = w_length/((float)(millis()-lastturn)/1000)*3.6;  //расчет скорости, км/ч
    lastturn = millis();  //запомнить время последнего оборота
    DIST = DIST+w_length/1000;  //прибавляем длину колеса к дистанции при каждом обороте оного
    DIST_1  = DIST_1+w_length/1000;
  }
}

void loop() {
  if ((micros()-lastturnkol)>1000000){ //если сигнала нет больше секунды
    RPM=0;  //считаем что RPM 0
  }
  if ((millis()-lastturn)>2000){ //если сигнала нет больше 2 секунды
    SPEED = 0;  //считаем что SPEED 0
    EEPROM.write(0,(float)DIST*10.0); //записываем DIST во внутреннюю память.  Также *10, чтобы сохранить десятую долю
  }  
  if (RPM < 600){kor_ygol = 3;}
  if ((RPM >= 600) && (RPM < 1000)){kor_ygol = 5;}
  if ((RPM >= 1000) && (RPM < 1500)){kor_ygol = 6.5;}
  if ((RPM >= 1500) && (RPM < 2000)){kor_ygol = 8;}
  if ((RPM >= 2000) && (RPM < 2500)){kor_ygol = 9.5;}
  if ((RPM >= 2500) && (RPM < 3000)){kor_ygol = 11;}
  if ((RPM >= 3000) && (RPM < 3500)){kor_ygol = 12.5;}
  if ((RPM >= 3500) && (RPM < 4000)){kor_ygol = 14;}
  if ((RPM >= 4000) && (RPM < 4500)){kor_ygol = 15.5;}
  if ((RPM >= 4500) && (RPM < 5000)){kor_ygol = 17;}
  if ((RPM >= 5000) && (RPM < 5500)){kor_ygol = 18.5;}
  if ((RPM >= 5500) && (RPM <= 6000)){kor_ygol = 20;}
  if (RPM > 6000){kor_ygol = 21;}
  
  ktime = (lastturnkol_1/360)*kor_ygol;
  t_ygol_dat = (lastturnkol_1/360)*ygol_dat;
  period = (t_ygol_dat - ktime)/1000;
    
      //Параметры для отображения
      //скорость
      cel_sp = floor(SPEED);  //целые
      sot_sp = (((float)cel_sp/1000)-floor((float)cel_sp/1000))*10; //сотни скорости
      des_sp = (((float)cel_sp/100)-floor((float)cel_sp/100))*10; //десятки скорости
      ed_sp = (((float)cel_sp/10)-floor((float)cel_sp/10))*10;  //единицы скорости
      //пробег
      cel_di = floor(DIST);  //целые
      dectus_di = (((float)cel_di/100000)-floor((float)cel_di/100000))*10;  //десятки тысяч расстояния
      tus_di = (((float)cel_di/10000)-floor((float)cel_di/10000))*10;  //тисячи расстояния
      sot_di = (((float)cel_di/1000)-floor((float)cel_di/1000))*10;  //сотни расстояния
      des_di = (((float)cel_di/100)-floor((float)cel_di/100))*10;  //десятки расстояния
      ed_di = floor(((float)((float)cel_di/10)-floor((float)cel_di/10))*10);  //единицы расстояния (с точкой)
      dr_di = (float)(DIST-floor(DIST))*10;  //десятые части расстояния
      //пробег за поездку
      cel_di_1 = floor(DIST_1);  //целые
      sot_di_1 = (((float)cel_di_1/1000)-floor((float)cel_di_1/1000))*10;  //сотни расстояния
      des_di_1 = (((float)cel_di_1/100)-floor((float)cel_di_1/100))*10;  //десятки расстояния
      ed_di_1 = floor(((float)((float)cel_di_1/10)-floor((float)cel_di_1/10))*10);  //единицы расстояния (с точкой)
      dr_di_1 = (float)(DIST_1-floor(DIST_1))*10;  //десятые части расстояния
      //обороты
      tus_ob = (((float)RPM/10000)-floor((float)RPM/10000))*10;  //тисячи расстояния
      sot_ob = (((float)RPM/1000)-floor((float)RPM/1000))*10;  //сотни расстояния
      des_ob = (((float)RPM/100)-floor((float)RPM/100))*10;  //десятки расстояния
      ed_ob = floor(((float)((float)RPM/10)-floor((float)RPM/10))*10);
      //температура
      sensors.requestTemperatures(); // Send the command to get temperatures
      t++;
      if (t=10){
        cel_t =  sensors.getTempCByIndex(0);
        t=0;
      }
      sot_t = (((float)cel_t/1000)-floor((float)cel_t/1000))*10;  //сотни температуры
      des_t = (((float)cel_t/100)-floor((float)cel_t/100))*10;  //десятки температуры
      ed_t = (((float)cel_t/10)-floor((float)cel_t/10))*10; //единицы температуры
      //вентелятор
      if(cel_t>=40){
        digitalWrite(rel_vent, HIGH);
        vent=true;
      }
      else{
        digitalWrite(rel_vent, LOW);
        vent=false;
      }
      //уровень топлива
      if (h=15){
        val = analogRead(analogPin);     // считываем значение c датчика затопления
        val_1 = map(analogRead(analogPin), 0, 450, 0, 100);
        h=0;
      }
      if(val>450){
        val=450;
      }
      //val_1 = (val/10)/2;
      val_1 = map(analogRead(analogPin), 0, 450, 0, 100);
      sot_val = (((float)val_1/1000)-floor((float)val_1/1000))*10;  //сотни процентов
      des_val = (((float)val_1/100)-floor((float)val_1/100))*10;  //десятки процентов
      ed_val = (((float)val_1/10)-floor((float)val_1/10))*10; //единицы процентов

      //ОТОБРАЖЕНИЕ
      //скорость
      lcd.setCursor(0, 0);
      lcd.print(sot_sp);
      lcd.print(des_sp);
      lcd.print(ed_sp);
      lcd.print("K");
      lcd.print("m");
      lcd.print("/");
      lcd.print("h");
      //пробег
      lcd.setCursor(0, 1);
      lcd.print(dectus_di);
      lcd.print(tus_di);
      lcd.print(sot_di);
      lcd.print(des_di);
      lcd.print(ed_di);
      lcd.print(".");
      lcd.print(dr_di);
      lcd.print("K");
      lcd.print("m");
      //температура
      lcd.setCursor(0, 2);
      //lcd.print(cel_t);
      lcd.print(sot_t);
      lcd.print(des_t);
      lcd.print(ed_t);
      lcd.write((byte) 0);
      lcd.print("C");
      //уровень топлива
      lcd.setCursor(13, 1);
      lcd.print(sot_val);
      lcd.print(des_val);
      lcd.print(ed_val);
      lcd.print("%");
      //обороты
      lcd.setCursor(13, 0);
      lcd.print(tus_ob);
      lcd.print(sot_ob);
      lcd.print(des_ob);
      lcd.print(ed_ob);
      lcd.print("r");
      lcd.print("p");
      lcd.print("m");
      //вентелятор
      lcd.setCursor(13, 2);
      if(vent==true)
      {
        lcd.print("Y");
        lcd.print("E");
        lcd.print("S");
      }
      if(vent==false)
      {
        lcd.print("N");
        lcd.print("O");
        lcd.print("T");
      }
      //пробег за поездку
      lcd.setCursor(0, 3);
      lcd.print(sot_di_1);
      lcd.print(des_di_1);
      lcd.print(ed_di_1);
      lcd.print(".");
      lcd.print(dr_di_1);
      lcd.print("K");
      lcd.print("m");
      p=0;
      Serial.print("RTM ");Serial.print(RPM);Serial.println(" ");
      Serial.print("Угол ");Serial.print(kor_ygol);Serial.println(" ");
}
