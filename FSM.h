#ifndef FSM_H
#define FSM_H

#include <Grove_LED_Bar.h>
#include "SwitchControl.h"

class State
{
public:
	enum StateID {
		STATE_NONE,
		STATE_OFF,
		STATE_STARTUP,
		STATE_PROGRAM,
		STATE_TIMER,
		STATE_WAIT,
		STATE_MAX
	};

	State(StateID state_id, SwitchControl &button, Grove_LED_Bar &led_bar) :
		state_id(state_id), button(button), led_bar(led_bar)
	    { /* fnord */ };

	virtual void enter() {
		state_start_time = millis();
	}

	virtual StateID update(SwitchControl::Event event);

	unsigned long state_time() {
		return millis() - state_start_time;
	};

	StateID get_id() {
		return state_id;
	}

protected:
	SwitchControl &button;
	Grove_LED_Bar &led_bar;

	StateID state_id;
	unsigned long state_start_time;
};


class OffState : public State
{
public:
	OffState(SwitchControl &button, Grove_LED_Bar &led_bar) : State(STATE_OFF, button, led_bar)
	    { /* fnord */ }

	void enter();

	StateID update(SwitchControl::Event event);
};


class StartupState : public State
{
public:
	StartupState(SwitchControl &button, Grove_LED_Bar &led_bar) : State(STATE_STARTUP, button, led_bar)
	    { /* fnord */ }

	void enter();

	StateID update(SwitchControl::Event event);
};


class ProgramState : public State
{
public:
	ProgramState(SwitchControl &button, Grove_LED_Bar &led_bar, unsigned long *total_time, unsigned long bar_time = 1800) : State(STATE_PROGRAM, button, led_bar),
		total_time(total_time), bar_time(bar_time), program_time(0)
	    { /* fnord */ }

	void enter();

	StateID update(SwitchControl::Event event);
private:
	unsigned long *total_time;
	unsigned long bar_time;
	unsigned long program_time;
};


class TimerState : public State
{
public:
	TimerState(SwitchControl &button, Grove_LED_Bar &led_bar, unsigned long *total_time) : State(STATE_TIMER, button, led_bar),
		total_time(total_time), last_update(0)
	    { /* fnord */ }

	void enter();

	StateID update(SwitchControl::Event event);
private:
	unsigned long *total_time;
	unsigned long last_update;
};


class WaitState : public State
{
public:
	WaitState(SwitchControl &button, Grove_LED_Bar &led_bar) : State(STATE_WAIT, button, led_bar)
	    { /* fnord */ }

	void enter();

	StateID update(SwitchControl::Event event);

private:
	void sweep_leds(int led, int dir, uint8_t *vals);

	unsigned long last_update;
	int sweep_led;
	int sweep_dir;
};


class Machine 
{
public:

	Machine() : current_state(State::StateID::STATE_NONE)
	    { /* fnord */ };
	
	void add_state(State *state);

	void update(SwitchControl::Event event);

	void set_state(State::StateID newstate);

private:
	State::StateID current_state;
	State *states[State::StateID::STATE_MAX];
};


#endif
