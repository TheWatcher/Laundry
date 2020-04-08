#include "FSM.h"

void Machine::update(SwitchControl::Event event)
{
	if (current_state != State::StateID::STATE_NONE) {
		set_state(states[current_state] -> update(event));
	}
}


void Machine::add_state(State *state)
{
	states[state -> get_id()] = state;
}


void Machine::set_state(State::StateID newstate)
{
	if (newstate != current_state) {
		current_state = newstate;
		states[current_state] -> enter();
	}
}


/* ============================================================================
 *  Base State 
 */

State::StateID State::update(SwitchControl::Event event)
{
	if (event == SwitchControl::EVENT_LONGPRESS) {
		return STATE_OFF;
	}

	return STATE_NONE;
}


/* ============================================================================
 *  STATE_OFF
 */

void OffState::enter()
{
	State::enter();

	led_bar.setLevel(0);
	button.set_led_state(false);
}

State::StateID OffState::update(SwitchControl::Event event)
{
	State::StateID newstate = State::update(event);
	if (newstate != STATE_NONE) {
		return newstate;
	}

	if (event == SwitchControl::EVENT_PRESSED) {
		return STATE_STARTUP;
	}

	return get_id();
}


/* ============================================================================
 *  STATE_STARTUP
 */

void StartupState::enter()
{
	State::enter();

	button.set_led_state(true);
}

State::StateID StartupState::update(SwitchControl::Event event)
{
	State::StateID newstate = State::update(event);
	if (newstate != STATE_NONE) {
		return newstate;
	}

	if (state_time() >= 1500) {
		return STATE_PROGRAM;
	} else {
		led_bar.setLevel((state_time() + 10) / 100);
	}
	
	return get_id();
}


/* ============================================================================
 *  STATE_PROGRAM
 */

void ProgramState::enter()
{
	State::enter();

	led_bar.setLevel(0);
	program_time = 1;
}

State::StateID ProgramState::update(SwitchControl::Event event)
{
	State::StateID newstate = State::update(event);
	if (newstate != STATE_NONE) {
		return newstate;
	}

	if (event == SwitchControl::EVENT_PRESSED) {
		++program_time;
		if (program_time > 10) {
			program_time = 1;
		}

		led_bar.setLevel(program_time);
	}

	unsigned long released = button.time_since_released();
	if (button.time_since_pressed() > 2000 && released > 2000) {
		if (((released - 2000) / 250) % 2) {
			led_bar.setLevel(0);
		} else {
			led_bar.setLevel(program_time);
		}

		if (released > 4500) {
			*total_time = program_time * (bar_time * 1000);
			return STATE_TIMER;
		}
	}

	return get_id();
}


/* ============================================================================
 *  STATE_TIMER
 */

void TimerState::enter()
{
	State::enter();

	last_update = 0;
	led_bar.setLevel(0);
}

State::StateID TimerState::update(SwitchControl::Event event)
{
	State::StateID newstate = State::update(event);
	if (newstate != STATE_NONE) {
		return newstate;
	}

	if(millis() > (last_update + 500)) {
		last_update = millis();

		led_bar.setLevel((float)state_time() / ((float)(*total_time) / 10.0f));
	}

	if (state_time() > *total_time) {
		return STATE_WAIT;
	}
	
	return get_id();
}


/* ============================================================================
 *  STATE_WAIT
 */

void WaitState::sweep_leds(int led, int dir, uint8_t *vals)
{
	uint8_t level = 0xff;

	dir *= -1;

	while (level > 0) {
		if (vals[led] == 0) {
			vals[led] = level;
		}

		led += dir;
		if (led < 0) {
			led = 0;
			dir = 1;
		}
		if (led > 9) {
			led = 9;
			dir = -1;
		}
		
		level /= 2;
	}
}

void WaitState::enter()
{
	State::enter();

	last_update = millis();
	sweep_led = 0;
	sweep_dir = 1;
	led_bar.setLevel(0);
}


State::StateID WaitState::update(SwitchControl::Event event)
{
	State::StateID newstate = State::update(event);
	if (newstate != STATE_NONE) {
		return newstate;
	}

	uint8_t leds[10] = { 0, 0, 0, 0, 0, 0, 0, 0, 0, 0 };

	if (event == SwitchControl::EVENT_PRESSED) {
		return STATE_STARTUP;
	}

	if (millis() > (last_update + 100)) {
		last_update = millis();

		sweep_led += sweep_dir;
		if (sweep_led == 9) {
			sweep_dir = -1;
		} else if (sweep_led == 0) {
			sweep_dir = 1;
		}
	
		sweep_leds(sweep_led, sweep_dir, leds);
		led_bar.setLeds(leds);
	}

	return get_id();
}
