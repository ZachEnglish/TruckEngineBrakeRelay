#include <PinChangeInterrupt.h>
#include <PinChangeInterruptBoards.h>
#include <PinChangeInterruptPins.h>
#include <PinChangeInterruptSettings.h>

#define TACH_PIN 2
#define RELAY_PIN 5
#define ROTARY_SWITCH 6
#define ROTARY_A 7
#define ROTARY_B 8

#include <Tachometer.h>
#include <Wire.h>
#include <LiquidCrystal_I2C.h>

LiquidCrystal_I2C lcd(0x27, 16, 2); // set the LCD address to 0x27 for a 16 chars and 2 line display

Tachometer tacho;

int g_rotary_value = 0;
int g_current_setting = 0;

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
}

void isr() {
  tacho.tick();
}

void rotarySwitchIsr() {
  g_current_setting++;

  if (g_current_setting > 5)
    g_current_setting = 0;
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
    lcd.print( (is_it_on ? "ON" : "OFF"));
    lcd.print(":");
    lcd.print(g_rotary_value);
    lcd.print(":");
    lcd.print(g_current_setting);
    
  }
}

bool set_relay_based_on_herts(float hz) {
  if (hz > 1000) {
    digitalWrite(RELAY_PIN, LOW);
    return true;
  }
  else {
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
      long increment = 1;

      if (newPinA == HIGH)  // must be HH now
      {
        if (pinA == LOW)
          g_rotary_value -= increment;
        else
          g_rotary_value += increment;
      }
      else
      { // must be LL now
        if (pinA == LOW)
          g_rotary_value += increment;
        else
          g_rotary_value -= increment;
      }
      ready = false;
    }  // end of being ready
  }  // end of completed click
  else
    ready = true;

  pinA = newPinA;
  pinB = newPinB;
}
