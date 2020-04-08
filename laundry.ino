#include <Grove_LED_Bar.h>
#include "SwitchControl.h"
#include "FSM.h"

// Configuration values for the peripherals
const int switch_pin = 2;
const int led_pin    = 3;
const int clock_pin  = 7;
const int data_pin   = 8;

// A variable to store the time the bar should fill over
unsigned long total_time = 0;

// The switch and led bar peripherals have objects to control them
SwitchControl control_switch(switch_pin, led_pin);
Grove_LED_Bar bar(clock_pin, data_pin, true, LED_BAR_10);

// Create the state objects the FSM will use, and the FSM itself
OffState state_off(control_switch, bar);
StartupState state_startup(control_switch, bar);
ProgramState state_program(control_switch, bar, &total_time);
TimerState state_timer(control_switch, bar, &total_time);
WaitState state_wait(control_switch, bar);
Machine fsm;

void setup() {

  // Ensure the switch and bar are in a sane initial state
  control_switch.setup();
  bar.begin();

  // Add the possible states to the FSM.
  fsm.add_state(&state_off);
  fsm.add_state(&state_startup);
  fsm.add_state(&state_program);
  fsm.add_state(&state_timer);
  fsm.add_state(&state_wait);
  fsm.set_state(State::StateID::STATE_OFF);
}

void loop() {
  // All the work of updating the bar is done in the FSM,
  // based on events generated by the control switch
	SwitchControl::Event event = control_switch.update();
  fsm.update(event);
}
