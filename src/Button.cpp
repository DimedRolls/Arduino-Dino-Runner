#include <Arduino.h>

class Button {
public:
  
  Button(int pin){

    _pin = pin;
    pinMode(pin, INPUT_PULLUP);

  }

  bool clickButton() {
  if (!digitalRead(_pin) && _itsClick == false && millis() - _timerBot > 250) {
    _itsClick = true;
    _timerBot = millis();
    return true;
  }
  if (digitalRead(_pin) && _itsClick == true) {
    _itsClick = false;
    return false;
  }
  }

private:

  int _pin = 0;                               //Пин на который подключена кнопка
  long int _timerBot = 0;                 //Ловим момент сробатывания кнопки для корректной работы таймера антидребезга
  bool _itsClick = 0;                     //Ловим нажатие кнопки

};