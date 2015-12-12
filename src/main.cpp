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
#define WRITABLE_WIDTH (LCD_WIDTH - 1)
#define WRITABLE_OFFSET 1

enum Button {
    RIGHT,
    UP,
    DOWN,
    LEFT,
    SELECT,
    NONE,
};

struct Monster {
    Monster* nextMonster;

    // space
    int position;

    // status
    int hunger;
};

struct Pen {
    Monster *firstMonster;
    bool treats[WRITABLE_WIDTH];
};

Pen pen;

// define some values used by the panel and buttons
Button last_button = NONE;
int last_button_pressed_at;
int cursor_position = 0;
int current_pen_index = 1;

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

void setup() {
    Serial.begin(9600);

    // initialize LCD screen
    lcd.begin(LCD_WIDTH, LCD_HEIGHT);

    // create custom characters
    lcd.createChar(SYMBOL_CARROT, symbol_carrot);
    lcd.createChar(SYMBOL_UNDERSCORE, symbol_underscore);

    // set up initial program state
    for (int i = 0; i < WRITABLE_WIDTH; ++i) {
        pen.treats[i] = 0;
        pen.firstMonster = (Monster*)malloc(sizeof(Monster));
        pen.firstMonster->position = 0;
    }
}

// read the buttons
Button readButtons() {
    int adc_key_in = analogRead(0);      // read the value from the sensor

    // LCD buttons when read are centered at values:
    //  Right: 0
    //  Up: 144
    //  Down: 329
    //  Left: 504
    //  Select: 741
    //
    // Add ~50 to those values since they are analog

    // check none first since it will be the most likely result
    if (adc_key_in >= 790) return NONE;
    if (adc_key_in <   50) return RIGHT;
    if (adc_key_in <  195) return UP;
    if (adc_key_in <  380) return DOWN;
    if (adc_key_in <  555) return LEFT;
    return SELECT; // >= 555, < 790
}

void printMonsters() {
    lcd.setCursor(0, TOP_LINE);
    char the_character[1];
    itoa(current_pen_index, the_character, 10);
    lcd.print(the_character);

    Monster* monster = pen.firstMonster;

    String positions = "               ";

    while (monster) {
        positions[monster->position] = 'a';
        monster = monster->nextMonster;
    }

    lcd.print(positions);
}

void printCursor() {
    lcd.setCursor(cursor_position + WRITABLE_OFFSET, BOTTOM_LINE);

    if (millis() / 500 % 2 == 0) {
        lcd.print(" ");
    } else {
        lcd.write(SYMBOL_UNDERSCORE);
    }
}

void printBottomLine() {
    lcd.setCursor(0, BOTTOM_LINE);
    lcd.write(SYMBOL_CARROT);

    for (int i = 0; i < WRITABLE_WIDTH; ++i) {
        if (pen.treats[i]) {
            lcd.write(SYMBOL_CARROT);
        } else {
            lcd.print(" ");
        }

    }
    printCursor();
}

void updateMonsters() {
    int new_position;
    Monster *monster = pen.firstMonster;

    while (monster) {
        monster->position = millis() / 1000 % WRITABLE_WIDTH;
        monster = monster->nextMonster;
    }
}

void processInput() {
    Button new_button = readButtons();

    if (new_button != last_button) {
        if (last_button == LEFT) {
            cursor_position = cursor_position - 1;
        } else if (last_button == RIGHT) {
            cursor_position = cursor_position + 1;
        } else if (last_button == SELECT) {
            pen.treats[cursor_position] = !pen.treats[cursor_position];
        }
        // make sure we are on the writable board
        cursor_position = (cursor_position + WRITABLE_WIDTH) % WRITABLE_WIDTH;
    }

    last_button = new_button;
}

void loop() {
    processInput();
    printMonsters();
    printBottomLine();
    updateMonsters();
}
