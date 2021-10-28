#include <PinChangeInterrupt.h>
#include <PinChangeInterruptBoards.h>
#include <PinChangeInterruptPins.h>
#include <PinChangeInterruptSettings.h>
#include <EEPROM.h>

#define TACH_PIN 2
#define RELAY_PIN 5
#define ROTARY_SWITCH 6
#define ROTARY_A 8
#define ROTARY_B 7

#include <Tachometer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

Tachometer tacho;

int g_rotary_setting_value = EEPROM.read(0) * 5;
int g_relay_shutoff_value = EEPROM.read(0) * 5;
int g_relay_switch_on_value = EEPROM.read(1) * 5;
int g_current_setting = 0;
bool g_screenValuesChanged;
bool in_exit = false;

int exit_time = 3; //these variables are used in function "void exit_settings()" and "void loop"
unsigned long last_number = 0;
unsigned long now_time;
unsigned long wait_time;

void setup()
{
  lcd.init();
  // Print a message to the LCD.
  lcd.backlight();

  pinMode(TACH_PIN, INPUT_PULLUP);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(ROTARY_SWITCH, INPUT_PULLUP);
  pinMode(ROTARY_A, INPUT_PULLUP);
  pinMode(ROTARY_B, INPUT_PULLUP);
  attachPinChangeInterrupt(digitalPinToPCINT(TACH_PIN), isr, FALLING); //This allows the Tachometer library to calculate the frequency of the incoming signal

  attachPinChangeInterrupt(digitalPinToPCINT(ROTARY_A), rotaryEncoderIsr, RISING);
  attachPinChangeInterrupt(digitalPinToPCINT(ROTARY_B), rotaryEncoderIsr, RISING);
  attachPinChangeInterrupt(digitalPinToPCINT(ROTARY_SWITCH), rotarySwitchIsr, RISING);

  g_screenValuesChanged = true;
}

void isr() {
  tacho.tick();
}

void rotarySwitchIsr() {
  if (in_exit == false) {
    g_current_setting++;

    g_screenValuesChanged = true; //tell arduino that these values have changed

    if (g_current_setting > 3)
      g_current_setting = 0;
  }
}

void loop() {

  if (g_screenValuesChanged == true) {
    status_display();
    g_screenValuesChanged = false; //tell arduino that we have displayed latest info
    settings();
  }

}

bool set_relay_based_on_herts(float hz) {
  if (hz > g_relay_switch_on_value) {
    digitalWrite(RELAY_PIN, LOW);
    return true;
  }
  if (hz < g_relay_shutoff_value) {
    digitalWrite(RELAY_PIN, HIGH);
    return false;
  }
}


// Interrupt Service Routine for rotary encoder.
// Stolen from Nick Gammon @ https://forum.arduino.cc/index.php?topic=62026.0
static void rotaryEncoderIsr()
{
  static boolean ready;
  static byte pinA, pinB;

  byte newPinA = digitalRead(ROTARY_A);
  byte newPinB = digitalRead(ROTARY_B);

  // Forward is: LH/HH or HL/LL
  // Reverse is: HL/HH or LH/LL

  // so we only record a turn on both the same (HH or LL)

  if (newPinA == newPinB)
  {
    if (ready)
    {
      long increment = 5;

      if (newPinA == HIGH)  // must be HH now
      {
        if (pinA == LOW)
          g_rotary_setting_value -= increment;
        else
          g_rotary_setting_value += increment;
      }
      else
      { // must be LL now
        if (pinA == LOW)
          g_rotary_setting_value += increment;
        else
          g_rotary_setting_value -= increment;
      }
      if (g_rotary_setting_value > 1200) //loop rotary encoder value from 1200 to 400 and visa versa
      {
        g_rotary_setting_value = 400;
      }
      if (g_rotary_setting_value < 400)
      {
        g_rotary_setting_value = 1200;
      }
      ready = false;
    }  // end of being ready
  }  // end of completed click
  else
    ready = true;

  pinA = newPinA;
  pinB = newPinB;

  g_screenValuesChanged = true; //tell arduino that the rotary values have been changed
}

void status_display() {
  float hz;
  bool is_it_on;
  if (g_current_setting == 0) {
    hz = tacho.getHz();
    is_it_on = set_relay_based_on_herts(hz);
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Brake Status: ");
    lcd.setCursor(0, 1);
    lcd.print( (is_it_on ? "ON" : "OFF"));
  }
}

void settings() {

  int eeprom_value;

  switch (g_current_setting) {
    case 0:
      break;
    case 1:
      g_relay_shutoff_value = g_rotary_setting_value;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Shut off Rpm: ");
      lcd.setCursor(0, 1);
      lcd.print(g_relay_shutoff_value);
      eeprom_value = g_relay_shutoff_value / 5;
      EEPROM.write(0, eeprom_value);
      break;
    case 2:
      g_relay_switch_on_value = g_rotary_setting_value;
      lcd.clear();
      lcd.setCursor(0, 0);
      lcd.print("Switch on Rpm: ");
      lcd.setCursor(0, 1);
      lcd.print(g_relay_switch_on_value);
      eeprom_value = g_relay_switch_on_value / 5;
      EEPROM.write(1, eeprom_value);
      break;
    case 3:
      in_exit = true;
      exit_settings();
      break;
  }
}

void exit_settings() {

  now_time = millis();

  wait_time = now_time - last_number;

  if (wait_time > 500) {
    lcd.clear();
    lcd.setCursor(0, 0);
    lcd.print("Exiting Settings: ");
    lcd.setCursor(0, 1);
    lcd.print(exit_time);
    exit_time -= 1;
    last_number = millis();
  }
  if (exit_time < 0) {
    exit_time = 3;
    g_current_setting = 0;
    in_exit = false;
  }

  g_screenValuesChanged = true;

}
