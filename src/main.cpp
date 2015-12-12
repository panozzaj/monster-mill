// include the standard Arduino functions
#include <Arduino.h>

// include the LCD library
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

// define some values used by the panel and buttons
int lcd_key = 0;
int adc_key_in = 0;
#define btnRIGHT  0
#define btnUP     1
#define btnDOWN   2
#define btnLEFT   3
#define btnSELECT 4
#define btnNONE   5

// symbols must be positive integers
#define SYMBOL_CARROT 1

byte symbol_carrot[8] = {
    B00000,
    B01010,
    B00100,
    B01110,
    B01110,
    B01110,
    B00100,
    B00000,
};

int monster_position = 0;

void setup() {
    Serial.begin(9600);

    // initialize LCD screen
    lcd.begin(16, 2);
    lcd.createChar(SYMBOL_CARROT, symbol_carrot);
}

// read the buttons
int read_lcd_buttons() {
    adc_key_in = analogRead(0);      // read the value from the sensor

    // LCD buttons when read are centered at values:
    //   0, 144, 329, 504, 741
    //
    // Add ~50 to those values since they are analog

    // check none first since it will be the most likely result
    if (adc_key_in >= 790) return btnNONE;
    if (adc_key_in < 50)   return btnRIGHT;
    if (adc_key_in < 195)  return btnUP;
    if (adc_key_in < 380)  return btnDOWN;
    if (adc_key_in < 555)  return btnLEFT;
    return btnSELECT; // >= 555, < 790
}

void printMonsters(int pen) {
  lcd.setCursor(0, 0);
  char theCharacter[1];
  itoa(pen, theCharacter, 10);
  lcd.print(theCharacter);

  for (int position = 0; position < 15; position++) {
      if (monster_position == position) {
          lcd.print("a");
      } else {
          lcd.print(" ");
      }
  }
}

void loop() {
    printMonsters(1);

    lcd.setCursor(0, 1);
    lcd.write(SYMBOL_CARROT);
    lcd_key = read_lcd_buttons();

    monster_position = (millis() / 500 % 15);
}
