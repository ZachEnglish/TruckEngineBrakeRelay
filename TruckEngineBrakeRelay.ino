#define TACH_PIN 2
#define RELAY_PIN 5

#include <Tachometer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>


LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

Tachometer tacho;

void setup()
{
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();
  
  pinMode(TACH_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  attachInterrupt(0, isr, FALLING); //This allows the Tachometer library to calculate the frequency of the incoming signal
}

void isr() {
  tacho.tick();
}

void loop()
{
  static uint32_t tmr;
  float hz;
  bool is_it_on;

  if (millis() - tmr > 1000) {
    tmr = millis();
    hz = tacho.getHz();
    is_it_on = set_relay_based_on_herts(hz);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Hz: ");
    lcd.print(hz);
    lcd.setCursor(0, 1);
    lcd.print("Relay: ");
    lcd.print( (is_it_on?"ON":"OFF"));
  }
}

bool set_relay_based_on_herts(float hz){
    if (hz > 1000) {
      digitalWrite(RELAY_PIN, LOW);
      return true;
    }
    else {
      digitalWrite(RELAY_PIN, HIGH);
      return false;
    }
}
