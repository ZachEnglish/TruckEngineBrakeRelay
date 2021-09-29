
#define TACH_PIN 2

#include <Tachometer.h>

//YWROBOT
//Compatible with the Arduino IDE 1.0
//Library version:1.1
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

Tachometer tacho;

void setup()
{
  lcd.init();                      // initialize the lcd
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  lcd.setCursor(3, 0);
  lcd.print("Hello, world!");
  lcd.setCursor(2, 1);
  lcd.print("Ywrobot Arduino!");
  lcd.setCursor(0, 2);
  lcd.print("Arduino LCM IIC 2004");
  lcd.setCursor(2, 3);
  lcd.print("Power By Ec-yuan!");

  pinMode(TACH_PIN, INPUT_PULLUP);
  pinMode(5, OUTPUT);
  attachInterrupt(0, isr, FALLING);
}

void isr() {
  tacho.tick();   // сообщаем библиотеке об этом
}

void loop()
{
  static uint32_t tmr;
  float herts;
  bool on;

  if (millis() - tmr > 1000) {
    tmr = millis();
    herts = tacho.getHz();
    on = set_relay_based_on_herts(herts);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Hz: ");
    lcd.print(herts);
    lcd.setCursor(0, 1);
    lcd.print("Relay: ");
    lcd.print( (on?"ON":"OFF"));
  }
}

bool set_relay_based_on_herts(float hz){
    if (hz > 1000) {
      digitalWrite(5, LOW);
      return true;
    }
    else {
      digitalWrite(5, HIGH);
      return false;
    }
}
