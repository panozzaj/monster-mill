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

// read the buttons
int read_LCD_buttons() {
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

void setup() {
    // initialize LCD screen
    lcd.begin(16, 2);

    lcd.setCursor(0, 0);
    lcd.print("Push the buttons"); // print a simple message
}

void loop() {
    lcd.setCursor(9, 1);
    lcd.print(millis() / 1000);

    lcd.setCursor(0, 1);
    lcd_key = read_LCD_buttons();

    switch (lcd_key) {
        case btnRIGHT:
            {
                lcd.print("RIGHT ");
                break;
            }
        case btnLEFT:
            {
                lcd.print("LEFT   ");
                break;
            }
        case btnUP:
            {
                lcd.print("UP    ");
                break;
            }
        case btnDOWN:
            {
                lcd.print("DOWN  ");
                break;
            }
        case btnSELECT:
            {
                lcd.print("SELECT");
                break;
            }
        case btnNONE:
            {
                lcd.print("NONE  ");
                break;
            }
    }
}
