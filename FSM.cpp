#include "FSM.h"

void Machine::update(SwitchControl::Event event)
{
    // If the FSM is in a sane state, with a known state impl, run the state's update.
    if (current_state != State::StateID::STATE_NONE && states[current_state] ) {
        set_state(states[current_state] -> update(event));
    }
}


void Machine::add_state(State *state)
{
    // Store the new state impl, potentially discarding any previous occupant of this slot
    states[state -> get_id()] = state;
}


void Machine::set_state(State::StateID newstate)
{
    if (newstate != current_state &&              // nothing to do if already in the right state
        newstate != State::StateID::STATE_NONE && // ignore attempts to go into no-state
        newstate <  State::StateID::STATE_MAX &&  // only allow states in the known range
        states[newstate]) {                       // and the state must have an implementation

        current_state = newstate;
        states[current_state] -> enter();
    }
}


/* ------------------------------------------------------------------------
 *  Base State
 */

State::StateID State::update(SwitchControl::Event event)
{
    // Longpresses always go to the off state, from any state.
    if (event == SwitchControl::EVENT_LONGPRESS) {
        return STATE_OFF;
    }

    // Otherwise, no change.
    return STATE_NONE;
}


/* ------------------------------------------------------------------------
 *  STATE_OFF
 */

void OffState::enter()
{
    State::enter();

    // Turn off the bar and button LEDs
    led_bar.setLevel(0);
    button.set_led_state(false);
}

State::StateID OffState::update(SwitchControl::Event event)
{
    State::StateID newstate = State::update(event);
    if (newstate != STATE_NONE) {
        return newstate;
    }

    // Wakeup from off on button press
    if (event == SwitchControl::EVENT_PRESSED) {
        return STATE_STARTUP;
    }

    return STATE_NONE;
}


/* ------------------------------------------------------------------------
 *  STATE_STARTUP
 */

void StartupState::enter()
{
    State::enter();

    // Turn on the button LED
    button.set_led_state(true);
}

State::StateID StartupState::update(SwitchControl::Event event)
{
    State::StateID newstate = State::update(event);
    if (newstate != STATE_NONE) {
        return newstate;
    }

    // fill in the LED bar based on the state time, with a bit of fudge on
    // the timer at the end so it shows all 10 for more than an instant
    if (state_time() >= 1500) {
        return STATE_PROGRAM;
    } else {
        led_bar.setLevel((state_time() + 10) / 100);
    }

    return STATE_NONE;
}


/* ------------------------------------------------------------------------
 *  STATE_PROGRAM
 */

void ProgramState::enter()
{
    State::enter();

    // There will always be a minimum of one bar turned on
    led_bar.setLevel(1);
    program_time = 1;
}

State::StateID ProgramState::update(SwitchControl::Event event)
{
    State::StateID newstate = State::update(event);
    if (newstate != STATE_NONE) {
        return newstate;
    }

    // If the user has pressed the button, increment the set time, with wrap
    if (event == SwitchControl::EVENT_PRESSED) {
        ++program_time;
        if (program_time > 10) {
            program_time = 1;
        }

        led_bar.setLevel(program_time);
    }

    // If the user hasn't pressed and released the button for a period,
    // look at flashing the LEDS or even starting the timer.
    unsigned long released = button.time_since_released();
     if (button.time_since_pressed() > hold_time && released > hold_time) {

        // Flash the LEDs on and off to indicate impending timer set
        if (((released - hold_time) / 250) % 2) {
            led_bar.setLevel(0);
        } else {
            led_bar.setLevel(program_time);
        }

        // If the user hasn't pressed anything for over the timeout time, set
        // the total time for the timer, and indicate the move to the new state
        if (released > timeout) {
            *total_time = program_time * (bar_time * 1000);
            return STATE_TIMER;
        }
    }

    return STATE_NONE;
}


/* ------------------------------------------------------------------------
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

    // Only update the bar every half second or so; even that's probably overkill
    if((unsigned long)(millis() - last_update) > 500) {
        last_update = millis();

        led_bar.setLevel((float)state_time() / ((float)(*total_time) / 10.0f));
    }

    // If we've been in the state long enough, switch to the wait state.
    if (state_time() > *total_time) {
        return STATE_WAIT;
    }

    return STATE_NONE;
}


/* ------------------------------------------------------------------------
 *  STATE_WAIT
 */

void WaitState::sweep_leds(int led, int dir)
{
    uint8_t leds[10];
    uint8_t level = 0xff;

    memset(leds, 0, 10);

    // We actually want to go in the opposite direction to the sweep dir,
    // as we're going to be building the 'trail' behind the head
    dir *= -1;

    while (level > 0) {
        // Do not overwrite alread-set LEDs - otherwise bounce trails
        // would overwrite the head!
        if (leds[led] == 0) {
            leds[led] = level;
        }

        // Move to the next LED
        led += dir;

        // Note that we need to explicitly handle out of bounds when doing
        // bounce here, as the += dir above can move the led to -1 or 10.
        if (led <= 0) {
            led = 0;
            dir = 1;
        }
        if (led >= 9) {
            led = 9;
            dir = -1;
        }

        level /= 3;
    }

    // Update the LED bar all in one go
    led_bar.setLeds(leds);
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

    if (event == SwitchControl::EVENT_PRESSED) {
        return STATE_STARTUP;
    }

    // Update the sweep every 10th of a second
    if ((unsigned long)(millis() - last_update) > 100) {
        last_update = millis();

        // Move to the next LED, 'bouncing' off the ends
        sweep_led += sweep_dir;
        if (sweep_led == 9) {
            sweep_dir = -1;
        }
        if (sweep_led == 0) {
            sweep_dir = 1;
        }

        sweep_leds(sweep_led, sweep_dir);
    }

    return STATE_NONE;
}
