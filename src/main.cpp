#include <Arduino.h>
#include "serial_keyboard.h"

SerialKeyboard sk;

void setup() {
    sk.begin();
}

void loop() {
    sk.update();
}