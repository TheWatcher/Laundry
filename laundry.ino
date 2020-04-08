#include <Grove_LED_Bar.h>
#include "SwitchControl.h"
#include "FSM.h"

const int switch_pin = 2;
const int led_pin    = 3;
const int clock_pin  = 7;
const int data_pin   = 8;

unsigned long total_time = 0;

SwitchControl control_switch(switch_pin, led_pin);
Grove_LED_Bar bar(clock_pin, data_pin, true, LED_BAR_10);

OffState state_off(control_switch, bar);
StartupState state_startup(control_switch, bar);
ProgramState state_program(control_switch, bar, &total_time);
TimerState state_timer(control_switch, bar, &total_time);
WaitState state_wait(control_switch, bar);

Machine fsm;

void setup() {
  Serial.begin(9600);
  control_switch.setup();
  bar.begin();

  fsm.add_state(&state_off);
  fsm.add_state(&state_startup);
  fsm.add_state(&state_program);
  fsm.add_state(&state_timer);
  fsm.add_state(&state_wait);
  fsm.set_state(State::StateID::STATE_OFF);
}

void loop() {
	SwitchControl::Event event = control_switch.update();
  fsm.update(event);
}
