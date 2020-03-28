#include <Grove_LED_Bar.h>
#include "SwitchControl.h"

const int switch_pin = 2;
const int led_pin    = 3;

SwitchControl control_switch(switch_pin, led_pin);

void setup() {
  // put your setup code here, to run once:
  control_switch.setup();
  Serial.begin(9600);
}

void loop() {
	SwitchControl::Event event = control_switch.update();

	if (event == SwitchControl::EVENT_PRESSED) {
		Serial.println("Pressed event");
    control_switch.set_led_state(true);
	} else if (event == SwitchControl::EVENT_LONGPRESS) {
		Serial.println("Long press event");
    control_switch.set_led_state(false);
	} else if (event == SwitchControl::EVENT_RELEASED) {
		Serial.println("Released event");
	}
}
