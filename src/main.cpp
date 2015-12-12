// include the standard Arduino functions
#include <Arduino.h>

// include the LCD library
#include <LiquidCrystal.h>

// initialize the library with the numbers of the interface pins
LiquidCrystal lcd(8, 9, 4, 5, 6, 7);

#define TOP_LINE 0
#define BOTTOM_LINE 1
#define LCD_WIDTH 16
#define LCD_HEIGHT 2

// define some values used by the panel and buttons
int last_key = 0;
int adc_key_in = 0;
int cursor_position = 0;

#define BUTTON_RIGHT  0
#define BUTTON_UP     1
#define BUTTON_DOWN   2
#define BUTTON_LEFT   3
#define BUTTON_SELECT 4
#define BUTTON_NONE   5

// symbols must be positive integers or cast as byte
#define SYMBOL_CARROT 1
#define SYMBOL_UNDERSCORE 2

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

byte symbol_underscore[8] = {
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B00000,
    B11111,
};

int monster_position = 0;

void setup() {
    Serial.begin(9600);

    // initialize LCD screen
    lcd.begin(LCD_WIDTH, LCD_HEIGHT);

    // create custom characters
    lcd.createChar(SYMBOL_CARROT, symbol_carrot);
    lcd.createChar(SYMBOL_UNDERSCORE, symbol_underscore);
}

// read the buttons
int read_lcd_buttons() {
    adc_key_in = analogRead(0);      // read the value from the sensor

    // LCD buttons when read are centered at values:
    //  Right: 0
    //  Up: 144
    //  Down: 329
    //  Left: 504
    //  Select: 741
    //
    // Add ~50 to those values since they are analog

    // check none first since it will be the most likely result
    if (adc_key_in >= 790) return BUTTON_NONE;
    if (adc_key_in < 50)   return BUTTON_RIGHT;
    if (adc_key_in < 195)  return BUTTON_UP;
    if (adc_key_in < 380)  return BUTTON_DOWN;
    if (adc_key_in < 555)  return BUTTON_LEFT;
    return BUTTON_SELECT; // >= 555, < 790
}

void printMonsters(int pen) {
  lcd.setCursor(0, TOP_LINE);
  char the_character[1];
  itoa(pen, the_character, 10);
  lcd.print(the_character);

  for (int position = 0; position < 15; position++) {
      if (monster_position == position) {
          lcd.print("a");
      } else {
          lcd.print(" ");
      }
  }
}

void printCursor() {
    lcd.setCursor(1, BOTTOM_LINE);

    if (millis() / 500 % 2 == 0) {
        lcd.print(" ");
    } else {
        lcd.write(SYMBOL_UNDERSCORE);
    }
}

void printBottomLine(int pen) {
    lcd.setCursor(0, BOTTOM_LINE);
    lcd.write(SYMBOL_CARROT);
    printCursor();
}

void updateMonsters() {
    monster_position = (millis() / 1000 % 15);
}

void loop() {
    printMonsters(1);
    printBottomLine(1);
    updateMonsters();
}
