//#define DEBAG
#include <Arduino.h>
#include <LiquidCrystal.h>
#include <Button.cpp>

Button but1 (13);                                //Непосредственно сами кнопки
Button but2 (12);
Button but3 (10);

void menu();                             //Прототип функции меню
void game();                             //Прототип функции игры

bool menuExit = 0;                       //Переменная для отключения меню
bool autoSpeed = 1;                      //Переменая для определения изменеия скорости игры

int skyLine[] = {0, 1, 0, 0, 1, 0, 1, 1, 0, 0}; //Отрисовка неба 8 элементов, чтобы оставить место под очки
int runLine[] = {0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0}; //Условная отрисовка земли
uint16_t speedRun = 600;                       //Скорость обновления экрана и игры


uint32_t timerRun = 0;                   //Таймер перерисовки монитора

byte playerAvatar1[8] = {                //Первый кадр бегуна
  0b00011,  0b00011,  0b10010,  0b01110,  0b00110,  0b01001,  0b11111,  0b00000
};

byte playerAvatar2[8] = {                //Второй кадр бегуна
  0b00011,  0b00010,  0b00011,  0b01110,  0b10110,  0b00110,  0b11111,  0b00000
};

byte playerAvatarFly[8] = {                //Полет бегуна
  0b00011,  0b00011,  0b00010,  0b01110,  0b10110,  0b00100,  0b01000,  0b00000
};

byte cactus1[8] = {                       //Кактус вариант 1
  0b00100,  0b00101,  0b10101,  0b10110,  0b01100,  0b00100,  0b11111,  0b00000
};

byte cactus2[8] = {                       //Кактус вариант 1
  0b00100,  0b10100,  0b10100,  0b01101,  0b00101,  0b00110,  0b11111,  0b00000
};

byte clouds[8] = {                       //Облака
  0b11111,  0b01110,  0b00000,  0b00110,  0b11111,  0b01100,  0b00000,  0b00000
};
byte clouds2[8] = {                      //Облака2
  0b11111,  0b01110,  0b00000,  0b0110,  0b11111,  0b0110,  0b00000,  0b00000
};

/* Создается объект для управления дисплеем.
   Пины ардуино присоеденены к дисплею
   в порядке: RS   E DB4 DB5 DB6 DB7*/
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);

void setup() {
#ifdef DEBAG
  Serial.begin(9600);                     //Открываем пор для отладки программы
#endif
  
  pinMode(9, OUTPUT);                     //Пин для пищалки

  randomSeed(analogRead(A0));             //Задаем случайное зерно рандома с "наводок воздуха"
  /* начинаем работу с экраном. Сообщаем объекту количество
    строк и столбцов. Опять же, вызывать pinMode не требуется:
    функция begin сделает всё за нас*/
  lcd.begin(16, 2);

  lcd.print("  =DinoChrom=  ");           // печатаем сообщение на первой строке

  lcd.createChar(1, playerAvatar1);       //Создаем свои символы это первый кадр бегуна
  lcd.createChar(2, playerAvatar2);       //Это второй кадр бегуна
  lcd.createChar(3, playerAvatarFly);     //Полет бегуна
  lcd.createChar(4, cactus1);             //Кактус 1
  lcd.createChar(5, cactus2);             //Кактус 2
  lcd.createChar(6, clouds);              //Облака
  lcd.createChar(7, clouds2);             //Облака2
  
#ifdef DEBAG
  Serial.println("Setup ower");
#endif
}

void loop() {
  if (menuExit == 0){  
    menu();
    }else{                     //Если игра запущена
    game();
  }
}

void menu (){

static uint8_t menuSelect = 0;

if (but2.clickButton()){              //По кнопке 2 выходим из настроек
  switch (menuSelect)
  {
    case 0:
    menuExit = 1;
    break;

    case 1:
    menuSelect = 2;
    break;
  
    case 2:
    menuSelect = 0;
    speedRun = 600;
    autoSpeed = 1;
    break;

    case 3:
    menuSelect = 4;
    break;
  
  default:
    menuSelect = 0;
    autoSpeed = 0;
    break;
  }
} 

if (but1.clickButton()){
  if (menuSelect != 1 && menuSelect != 3 && menuSelect != 4){
    menuSelect = ++menuSelect;
  }else if (menuSelect == 4 && speedRun < 900){//По кнопке 1 увеличиваем скорость бегуна 
    speedRun += 50;
  }
}
if (but3.clickButton()){
if (menuSelect != 0 && menuSelect != 2 && menuSelect != 4){
    menuSelect = --menuSelect;
  }else if (menuSelect == 4 && speedRun > 300){//По кнопке 3 уменьшаем скорость бегуна 
        speedRun -= 50;
  }
}

if (millis() - timerRun > 500){ //Обнавляем меню раз в секунду и отключаем его во время игры

  lcd.setCursor(0,1);
  lcd.print("Button 2 - Select");
  lcd.setCursor(0,0);
  
  switch (menuSelect){                       //Выбераем пункт меню для отрисовки
    case 0:
      lcd.print("  Start game >> ");          
    break;
  
    case 1:
      lcd.print(" << Settings    ");
    break;

    case 2:
      lcd.print("Speed run: Auto>");
    break;
    
    case 3:
      lcd.print("Speed run: Set <");
    break;

    case 4:
      lcd.print("Speed run: ");
      lcd.print(speedRun);
    break;
    
    }
  }
}

void game (){

static bool jump = 0;                           //Прыжок Дино бегуна
static bool setC = 1;                           //Флажок установки курсора вовторую строку
static bool gameOwer = 0;
static bool pause = 0;

static uint16_t score = 0;                           //Победные очки (кадр/сек)
static int flight = 0;                          //Счетчик кадров полета бегуна
static int stepPlyer = 0;                       //Счетчик шагов для анимации бега
static int saveChar = 0;                         //Здесь хранится часть поля перед дино, корректной отрисовки поля за ним при прохождении поля

  if (but1.clickButton() && gameOwer != 1) {   //Опрашиваем кнопку1 на нажатие пока активна игра
    pause = !pause;                       //Если кнопка нажата ставим игру на паузу
    lcd.setCursor(0,0);
    lcd.print("  Pause  ");
  } 
  if (but2.clickButton() && skyLine[3] == 0 && pause == 0) { //Опрашиваем кнопку2 на нажатие
    jump = 1;          //Если кнопка нажата то активируем прыжок
  }
 
 
  if (millis() - timerRun > speedRun) {       //Каждую секунду обнавляем игру(экран, положение дино и тд)
    if (gameOwer == 0 && pause == 0) {
#ifdef DEBAG
      Serial.print("Click: ");              //Отладочный код
      Serial.println(bot2.itsClick);        //Отладочный код
      Serial.print("Timer: ");              //Отладочный код
      Serial.println(bot2.timerBot);        //Отладочный код
#endif
      for (int s = 0; s != 10; s++) {       //Обнавляем небо игровго поля

        if (s != 9) {                       // в 9 ячейке будет появляться новая часть неба

          if (s == 2 && skyLine[3] == 3) {      //Проверяем положение дино и записываем ячейку поля после него
            skyLine[s] = random(0, 2);
          } else if (s == 3 && skyLine[s] == 3) {
            skyLine[s] = 3;
          } else {
            skyLine[s] = skyLine[s + 1];
          }
  
        } else {
          skyLine [s] = random(0, 2);       //выбераем случайное облако
        }
      }
      
      for (int s = 0; s != 16; s++) {       //Обнавляем игровое поле

        if (s != 15) {                      // в 15 ячейке будет появляться новая часть земли

          if (s == 2 && flight == 0) {      //Проверяем положение дино и записываем ячейку поля после него
            runLine[s] = saveChar;
          } else {
            runLine[s] = runLine[s + 1];
          }
        } else if (runLine [s - 2] == 1 || runLine [s - 2] == 2) { //исключаем появление 3х и более препятствий
          runLine[s] = 0;
        } else {
          runLine [s] = random(0, 3);       //выбераем случайное препятствие или пустую ячейку
        }
      }

      saveChar = runLine[3];              //Записываем ячейку в которой отрисовываеться динозавр.

      if (jump == 1 && flight == 0) {       //Определяем положение дино если начало прыжка
        skyLine[3] = 3;                     //Переносим дино в воздух
        tone(9, 1200, 100);                 //Звук прыжка
#ifdef DEBAG
        Serial.println("JUMP!");            //Отладочный код
#endif
        flight = 1;                         //Зависаем в воздухе
        jump = 0;                           //Приземление
      } else if (flight == 0) {             //Пока летим
        skyLine[3] = 0;                     //Убираем с неба
        runLine[3] = 3;                     //Посадка дино
      } else {                              //Приземление
        flight = flight - 1;                //Потихоньку приземляемся
#ifdef DEBAG
        Serial.print("I: ");
        Serial.println(flight);             //Отладочный код
#endif
      }
#ifdef DEBAG
      Serial.print("Dino: ");
      Serial.println(runLine [4]);             //Отладочный код
      Serial.print("plase: ");
      Serial.println(saveChar);             //Отладочный код
#endif

      if (jump == 0 && flight == 0) {
        if ((saveChar == 1 || saveChar == 2) && runLine [3] == 3) {
          gameOwer = 1;
        }
      }

      lcd.clear();
      //lcd.setCursor(0, 0);                         //Вывод поля

      for (int f = 0; f != 26; f++) {
        if (f < 10) {
          if (skyLine[f] == 0) {
            lcd.write(byte(6));
          } else if (skyLine[f] == 1) {
            lcd.write(byte(7));
          } else if (skyLine[f] == 3) {
            lcd.write(byte(3));
          }
        } else {
          if (setC == 1) {
            lcd.setCursor(0, 1);
            setC = 0;
          }
          if (runLine[f - 10] == 0) {
            lcd.write('_');
          } else if (runLine[f - 10] == 1) {
            lcd.write(byte(4));
          } else if (runLine[f - 10] == 2) {
            lcd.write(byte(5));
          } else if (runLine[f - 10] == 3 && stepPlyer == 0) {
            lcd.write(byte(1));
            stepPlyer = 1;
            tone(9, 500, 50);
          } else if (runLine[f - 10] == 3 && stepPlyer == 1) {
            lcd.write(byte(2));
            stepPlyer = 0;
            tone(9, 700, 50);
          }
        }
      }
      lcd.setCursor(10, 0);
      setC = 1;
#ifdef DEBAG
      Serial.println(score);
#endif
      lcd.print("Sc:");
      lcd.print(score);
      if (gameOwer != 1){
        score = ++score;
      }
      timerRun = millis();

      if (autoSpeed == 1 && (score%50 == 0) && speedRun > 300)
      {
        speedRun -= 100;
      }

    }
    
    if (gameOwer == 1){
      lcd.setCursor(0, 0);
      lcd.print("Game Ower!Sc:");
      lcd.print(score);
      lcd.setCursor(5, 1);
      lcd.print("Restart Y/N");
          
      jump = 0;                             //Прыжок Дино бегуна
      setC = 1;                             //Флажок установки курсора вовторую строку
      flight = 0;                           //Счетчик кадров полета бегуна
      saveChar = 0;

      for (int s = 0; s != 16; s++) {       //Обнавляем игровое поле
        if (s < 7) {                      // в 15 ячейке будет появляться новая часть земли
          runLine[s] = 0;
        } else if (runLine [s - 2] == 1 || runLine [s - 2] == 2) { //исключаем появление 3х и более препятствий
          runLine[s] = 0;
        } else {
          runLine [s] = random(0, 3);       //выбераем случайное препятствие или пустую ячейку
        }
      }
    
      if (autoSpeed == 1)
        {
          speedRun = 600;
        }
      if (but3.clickButton()){                //Опрашиваем кнопку 3 на нажатие
        gameOwer = 0;                         //Если кнопка нажата рестартим игру
        lcd.setCursor(13,0);
        lcd.print("   ");
        score = 0;
      }
      if (but2.clickButton()){                //Опрашиваем кнопку 2 на нажатие
        gameOwer = 0;                         //Если кнопка нажата рестартим игру 
        menuExit = 0;                         //и переходим в меню
        score = 0;                        
        lcd.setCursor(13,0);
        lcd.print("   ");
    }
    }
  }
}