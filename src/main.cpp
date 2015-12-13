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

// game speed variables
#define SECONDS_FOR_HUNGER 500
#define DEATH_HUNGER 20

#define INPUT_MODE_TREAT 0
#define INPUT_MODE_MONEY 1
#define INPUT_MODES 2

enum Button {
    RIGHT,
    UP,
    DOWN,
    LEFT,
    SELECT,
    NONE,
};

enum Species {
    FUZZBALL,
    DRAGON,
};

struct Monster {
    unsigned int id;
    Monster* next_monster;
    Monster* previous_monster;
    Species species;
    bool alive;

    // space
    signed char position;

    // actions
    unsigned long last_acted_millis;
    unsigned long last_hunger_millis;
    unsigned long died_at_millis;
    unsigned char speed;

    // status
    int hunger;
};

struct Pen {
    Monster *first_monster;
    bool treats[WRITABLE_WIDTH];
};

Pen pen;

// define some values used by the panel and buttons
Button last_button = NONE;
signed char cursor_position = 0;
int current_pen_index = 1;
int bank_balance = 100;
char input_mode = INPUT_MODE_TREAT;

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

Monster* createMonster(char position) {
    Monster* monster = (Monster*)malloc(sizeof(Monster));
    monster->position = position;
    monster->id = 0;
    monster->species = FUZZBALL;
    monster->speed = 3;
    monster->hunger = 5;
    monster->alive = 1;
    monster->last_acted_millis = millis();
    monster->last_hunger_millis = millis();
    monster->born_at_millis = millis();
    monster->previous_monster = NULL;
    monster->next_monster = NULL;
    return monster;
}

void setup() {
    Serial.begin(9600);
    randomSeed(analogRead(0));

    // initialize LCD screen
    lcd.begin(LCD_WIDTH, LCD_HEIGHT);

    // create custom characters
    lcd.createChar(SYMBOL_CARROT, symbol_carrot);
    lcd.createChar(SYMBOL_UNDERSCORE, symbol_underscore);

    // set up initial program state
    for (unsigned char i = 0; i < WRITABLE_WIDTH; ++i) {
        pen.treats[i] = 0;
    }

    pen.first_monster = createMonster(2);
    Monster* monster1 = createMonster(6);
    Monster* monster2 = createMonster(10);

    pen.first_monster->next_monster = monster1;
    monster1->previous_monster = pen.first_monster;
    monster1->next_monster = monster2;
    monster2->previous_monster = monster1;
}

// read the buttons
Button readButtons() {
    // read the value from the sensor
    short int adc_key_in = analogRead(0);

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
    char pen_char[1];
    itoa(current_pen_index, pen_char, 10);
    lcd.print(pen_char);

    Monster* monster = pen.first_monster;

    // TODO: change to be able to print custom characters
    String positions = "               ";

    while (monster != NULL) {
        if (!monster->alive) {
            positions[monster->position] = 'x';
        } else {
            if (monster->species == FUZZBALL) {
                positions[monster->position] = 'f';
            } else if (monster->species == DRAGON) {
                positions[monster->position] = 'D';
            } else {
                positions[monster->position] = '?';
            }
        }

        monster = monster->next_monster;
    }

    lcd.print(positions);
}

void printCursor() {
    lcd.setCursor(cursor_position + WRITABLE_OFFSET, BOTTOM_LINE);

    if (millis() / 500 % 2 == 0) {
        lcd.write(SYMBOL_UNDERSCORE);
    }
}

void printBottomLine() {
    switch (input_mode) {
        case INPUT_MODE_TREAT:
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
            break;

        case INPUT_MODE_MONEY:
            lcd.setCursor(0, BOTTOM_LINE);
            lcd.print("$");
            lcd.print(bank_balance);
            lcd.print("               ");
            break;

        default:
            lcd.setCursor(0, BOTTOM_LINE);
            lcd.print("?");
            break;
    }
}

void updateMonsters() {
    Monster *monster = pen.first_monster;

    while (monster) {
        unsigned long current_millis = millis();

        // slowest action is once every four seconds, depends on speed
        unsigned long next_action_millis =
            monster->last_acted_millis + 4000L / monster->speed;

        // Increase hunger, modified by monster's base metabolism.
        unsigned long next_hunger_millis =
            monster->last_hunger_millis +
            SECONDS_FOR_HUNGER * 1000L / monster->speed;

        bool acted = 0;

        if (monster->alive) {
            if (current_millis >= next_hunger_millis) {
                monster->hunger++;
                if (monster->hunger >= DEATH_HUNGER) {
                    monster->alive = 0;
                    monster->died_at_millis = current_millis;
                }
                monster->last_hunger_millis = next_hunger_millis;
            }
        }

        if (monster->alive) {
            if (current_millis >= next_action_millis) {
                if (monster->hunger > 0 && pen.treats[monster->position] && !acted) {
                    // eat the treat
                    pen.treats[monster->position] = 0;
                    monster->hunger--;
                    acted = 1;
                }

                if (!acted) {
                    monster->position += random(2) * 2 - 1;
                    monster->position += WRITABLE_WIDTH;
                    monster->position %= WRITABLE_WIDTH;
                    acted = 1;
                }

                // use current_millis instead?
                monster->last_acted_millis = next_action_millis;
            }

            monster = monster->next_monster;
        } else { // dead
            if (current_millis > monster->died_at_millis + 10 * 1000) {
                Monster* next = monster->next_monster;
                Monster* prev = monster->previous_monster;

                // remove monster from ecosystem
                // NULL - monster - NULL -> nothing
                // prev - monster - NULL -> prev - NULL
                // NULL - monster - next -> NULL - next
                // prev - monster - next -> prev - next

                if (prev != NULL) {
                    prev->next_monster = next;
                }

                if (next != NULL) {
                    prev->previous_monster = prev;
                }

                if (pen.first_monster == monster) {
                    pen.first_monster = next;
                }

                free(monster);
                monster = next;
            } else {
                monster = monster->next_monster;
            }
        }
    }
}

void processInput() {
    Button new_button = readButtons();

    if (new_button != last_button) {
        if (last_button == LEFT) {
            cursor_position--;
        } else if (last_button == RIGHT) {
            cursor_position++;
        } else if (last_button == UP) {
            input_mode -= 1;
        } else if (last_button == DOWN) {
            input_mode += 1;
        } else if (last_button == SELECT) {
            if (pen.treats[cursor_position]) {
                // remove treat
                pen.treats[cursor_position] = 0;
                bank_balance++;
            } else {
                if (bank_balance > 0) {
                    pen.treats[cursor_position] = 1;
                    bank_balance--;
                }
            }
        }

        // make sure we are positive

        input_mode += INPUT_MODES;
        input_mode %= INPUT_MODES;

        cursor_position += WRITABLE_WIDTH;
        cursor_position %= WRITABLE_WIDTH;
    }

    last_button = new_button;
}

void loop() {
    processInput();
    printMonsters();
    printBottomLine();
    updateMonsters();
}
