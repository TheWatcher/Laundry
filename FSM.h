#ifndef FSM_H
#define FSM_H

#include <Grove_LED_Bar.h>
#include "SwitchControl.h"

/** The base class for states in the Finite State Machine. This implements
 *  the core functionality common to all states, and each state derived
 *  from it should override the enter() and update() functions to implement
 *  state-specific behaviours.
 */
class State
{
public:
    /** A list of know state IDs. Each state should have a unique ID
     *  in this enum, and STATE_MAX must always be at the end of the enum.
     */
    enum StateID {
        STATE_NONE,    //!< Dummy state, required to have a sane default in the FSM.
        STATE_OFF,
        STATE_STARTUP,
        STATE_PROGRAM,
        STATE_TIMER,
        STATE_WAIT,
        STATE_MAX      //!< A convenience value used to track how many states there are.
    };

    /** Create a new state. Each state may need to interact with either the
     *  control switch or the LED bar, so references to each are held onto by
     *  the base state.
     *
     * @param state_id  The ID of the state being created.
     * @param button    A refrence to a button peripheral control object.
     * @param led_bar   A reference to a LED bar control object.
     * @return A new State object.
     */
    State(StateID state_id, SwitchControl &button, Grove_LED_Bar &led_bar) :
        state_id(state_id), button(button), led_bar(led_bar)
        { /* fnord */ };


    /** Actions to take when entering a state. Generally derived states should call
     *  this function in the base state to ensure that the state timer is set, and
     *  then perform additional state-specific setup.
     */
    virtual void enter() {
        state_start_time = millis();
    }


    /** Perform any updates required in the current state. In the base state,
     *  this checks whether the switch has been long pressed to indicate that
     *  the system should go back to the off state. Implementations in derived
     *  classes should call the base update, and if it returns anything other
     *  than 'STATE_NONE' they should immediately return that new state.
     *
     *  @param event The most recent event from the control switch.
     *  @return The next state to move to in the FSM, or STATE_NONE to
     *          indicate to the caller that no change is needed.
     */
    virtual StateID update(SwitchControl::Event event);


    /** Obtain the time that the state has been active.
     *
     * @return The amount of time the state has been active, in milliseconds.
     */
    unsigned long state_time() {
        return millis() - state_start_time;
    };


    /** Obtain the ID of the state.
     *
     * @return The state's ID.
     */
    StateID get_id() {
        return state_id;
    }

protected:
    SwitchControl &button;  //!< A reference to the button peripheral control object
    Grove_LED_Bar &led_bar; //!< A reference to the LED bar control object

    StateID state_id;               //!< The ID for the state
    unsigned long state_start_time; //!< The time at which the state started, in millis
};


/** Derived class implementing the STATE_OFF state. This simply ensures that
 *  the LED bar is off, and the control switch light is off, and waits for
 *  a button press event to move from STATE_OFF to STATE_STARTUP.
 */
class OffState : public State
{
public:
    OffState(SwitchControl &button, Grove_LED_Bar &led_bar) : State(STATE_OFF, button, led_bar)
        { /* fnord */ }

    void enter();

    StateID update(SwitchControl::Event event);
};

/** Derived class implementating the STATE_STARTUP state. This turns on the
 *  control button LED, and fills in the LED bar one element at at time as a
 *  self-test. Once the bar has been filled, the update() function tells the
 *  state machine to move to the STATE_PROGRAM state.
 */
class StartupState : public State
{
public:
    StartupState(SwitchControl &button, Grove_LED_Bar &led_bar) : State(STATE_STARTUP, button, led_bar)
        { /* fnord */ }

    void enter();

    StateID update(SwitchControl::Event event);
};


/** Derived class implementing the STATE_PROGRAM state. In this state, button
 *  presses by the user increase the number of lit bars in the LED bar, with
 *  each lit bar corresponding to a period of time the system should spend in
 *  the STATE_TIMER state (as determined by the 'bar_time' variable). If the
 *  bar is filled, pressing the button again makes it wrap around to one bar.
 *  If the user does not press the button for more than `hold_time` milli,
 *  the selected bar elements flash on and off to indicate that the timer will
 *  be set soon, and after `timeout` milis the update() function tells the
 *  state machine to move to the STATE_TIMER state.
 */
class ProgramState : public State
{
public:
    /** Create a new ProgramState object. Along with the TimerState constructor,
     *  this state constructor requires additional arguments - in particular, it
     *  must be given a pointer to an unsigned long that can be shared with the
     *  TimerState to pass the time the user has selected from the ProgramState
     *  to the TimerState.
     *
     * @param button     A refrence to a button peripheral control object.
     * @param led_bar    A reference to a LED bar control object.
     * @param total_time A pointer to a variable used to share the selected time
     *                   with the TimerState state.
     * @param bar_time   How much time, in seconds, each bar adds to the time.
     */
    ProgramState(SwitchControl &button, Grove_LED_Bar &led_bar, unsigned long *total_time, unsigned long bar_time = 1800) : State(STATE_PROGRAM, button, led_bar),
        total_time(total_time), bar_time(bar_time), program_time(0)
        { /* fnord */ }

    void enter();

    StateID update(SwitchControl::Event event);
private:
    static const unsigned long hold_time = 2000; //!< Delay from last release before flashing the selected bars
    static const unsigned long timeout   = 4500; //!< Delay from last release before switching to timer state

    unsigned long *total_time;  //!< A pointer to a variable used to share the selected time with the TimerState state.
    unsigned long bar_time;     //!< How much time, in seconds, each bar adds to the time.
    unsigned long program_time; //!< How many bars the user has selected as the programmed time
};


class TimerState : public State
{
public:
    /** Create a new TimerState object. Along with the ProgramState constructor,
     *  this state constructor requires additional arguments - in particular, it
     *  must be given a pointer to an unsigned long that can be shared with the
     *  ProgramState to pass the time the user has selected from the ProgramState
     *  to the TimerState.
     *
     * @param button     A refrence to a button peripheral control object.
     * @param led_bar    A reference to a LED bar control object.
     * @param total_time A pointer to a variable containing the time set by the
     *                   ProgramState, in millis
     */
    TimerState(SwitchControl &button, Grove_LED_Bar &led_bar, unsigned long *total_time) : State(STATE_TIMER, button, led_bar),
        total_time(total_time), last_update(0)
        { /* fnord */ }

    void enter();

    StateID update(SwitchControl::Event event);
private:
    unsigned long *total_time; //!< A pointer to a variable containing the time set by the ProgramState, in millis
    unsigned long last_update; //!< The last time the display was updated, in millis
};


class WaitState : public State
{
public:
    WaitState(SwitchControl &button, Grove_LED_Bar &led_bar) : State(STATE_WAIT, button, led_bar)
        { /* fnord */ }

    void enter();

    StateID update(SwitchControl::Event event);

private:
    /** Display a LED with a fading 'trail' on the LED bar. This sets the led
     *  at the specified position to full brightness, and then builds a trail
     *  of decreasing brightness behind the LED in the opposite direction to
     *  the specified direction.
     *
     * @param led The LED to set to full brightness, range 0 to 9
     * @param dir The direction the brightest LED is 'moving', should be -1 or 1
     */
    void sweep_leds(int led, int dir);

    unsigned long last_update;  //!< The last time the display was updated, in millis
    int sweep_led;              //!< Which LED is currently the 'head' of the sweep (0 to 9)
    int sweep_dir;              //!< Which direction the sweep is currently going (-1 or 1)
};


/** A very basic finite state machine implementation. This class does not
 *  do anything fancy involving internal/external events, it just keeps
 *  track of possible states and which state is current, and relies on the
 *  state implementation update() functions to determine which state the
 *  machine should move to.
 */
class Machine
{
public:
    /** Create a new, empty finite state machine. Before the state machine
     *  can be used for anything useful, states must be added using the
     *  add_state() function, and the initial state selected using set_state().
     *
     * @return A new state machine object.
     */
    Machine() : current_state(State::StateID::STATE_NONE)
        { /* fnord */ };

    /** Add a new state implementation to the state machine. If a state
     *  implementation with the same ID is already in the FSM, it will
     *  be replaced.
     *
     * @param state A pointer to a state implementation to add to the machine.
     */
    void add_state(State *state);


    /** Update the state machine. This will invoke the update function for the
     *  current state, and potentially move the state machine into a new state.
     *
     * @param event The last event generated by the button peripheral.
     */
    void update(SwitchControl::Event event);


    /** Update the current state of the state machine, if needed. This will
     *  move the state machine into the specified state, if it is not already
     *  in that state, and the specified state is a valid, implemented state.
     *
     * @param newstate The ID of the new state to move the machine to.
     */
    void set_state(State::StateID newstate);

private:
    State::StateID current_state;             //!< The ID of the current state of the machine
    State *states[State::StateID::STATE_MAX]; //!< Storage for pointers to implementation objects for each state
};


#endif
