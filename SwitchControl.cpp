#include "SwitchControl.h"

void SwitchControl::setup()
{
	pinMode(switch_pin, INPUT);
	pinMode(led_pin, OUTPUT);

	// Explicitly set the LED to a known (off) state.
	set_led_state(false);
}


void SwitchControl::set_led_state(bool state)
{
	digitalWrite(led_pin, state ? HIGH : LOW);
}


SwitchControl::Event SwitchControl::update()
{
	Event event = EVENT_NONE;

	uint8_t current_state = digitalRead(switch_pin);

	// If the state has changed since the last update, reset the debounce timer
	if(current_state != last_state) {
		last_debounce = millis();
	}

	// If the debounce timer has been going for longer than the debounce time,
	// a valid state change might be present
	if((millis() - last_debounce) > debounce_time) {
		
		// If the state has changed, update
		if(current_state != switch_state) {
			switch_state = current_state;

			// Convert the switch status into an event type and record the time
			if(switch_state == HIGH) {
				last_press = millis();
				event = EVENT_PRESSED;
			} else {
				in_longpress = false;    // by definition, can't be in longpress if released.
				last_release = millis();
				event = EVENT_RELEASED;
			}
		}

		// Has the switch been held down for more than the longpress time?
		if(!in_longpress && switch_state == HIGH && ((millis() - last_press) > longpress_time)) {
			in_longpress = true;
			event = EVENT_LONGPRESS;
		}
	}

	// Record the current state for comparison next update()
	last_state = current_state;

	return event;
}