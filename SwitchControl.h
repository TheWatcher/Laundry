#ifndef SwitchControl_H
#define SwitchControl_H

#include <Arduino.h>

class SwitchControl
{
public:
	/** The possible kinds of events that may be reported by update()
	 */
	enum Event {
		EVENT_NONE,      //!< Nothing happened. Nothing to see here, move along.
		EVENT_PRESSED,   //!< The switch was pressed.
		EVENT_LONGPRESS, //!< The switch had been held long enough to trigger a longpress.
		EVENT_RELEASED   //!< The switch was released.
	};

	/** Create a new SwitchControl object for interacting with an illuminated
	 *  push-button switch.
	 *
	 * @param switch_pin The digital pin the switch is connected to. This should 
	 *                   go high when the switch is pressed, and low when not.
	 * @param led_pin    The digital pin the LED is connected to. The LED will be 
	 *                   initialised to be off during setup.
	 * @param debounce_time Time in milliseconds to delay swith state change by to
	 *                   allow for switch bounce to be ignored. Increase this if 
	 *                   spurious press and release events are generated.
	 * @param longpress_time If the switch is held pressed for this amount of time
	 *                   in milliseconds a 'long press' event will be generated.
	 */
	SwitchControl(uint8_t switch_pin, uint8_t led_pin, unsigned long debounce_time = 50, unsigned long longpress_time = 3000) : 
		switch_pin(switch_pin), led_pin(led_pin), 
		switch_state(LOW),in_longpress(false),last_press(0),last_release(0),
		last_state(LOW),last_debounce(0),
		debounce_time(debounce_time), longpress_time(longpress_time)
	{ /* fnord */ }
	

	/* ------------------------------------------------------------------------
	 *  Setup and main loop interaction
	 */

	/** Initialise the IO for the SWitchControl object. 
	 *  This sets the configuration of the IO pins for the switch and LED control,
	 *  it should be called once from the global setup() function.
	 */
	void setup();


	/** Check the status of the switch, and determine whether any events
	 *  should be triggered as a result of its state. This performs switch 
	 *  debouncing to try to avoid spurious events, and can detect when the switch
	 *  has been held down to trigger a long press event.
	 *
	 * @return A value indicating whether an event happened during this
	 *         update, and if so what kind of event.
	 */
	Event update();

	/* ------------------------------------------------------------------------
	 *  Control functins
	 */

	/** Set the LED in the switch to either on or off.
	 * 
	 * @param state Set to `true` to turn the LED on, `false` to turn it off.
	 */
	void set_led_state(bool state);


	/* ------------------------------------------------------------------------
	 *  State lookup
	 */

	/** Determine whether the switch is currently pressed.
	 *
	 * @return `true` if the switch is currently pressed, `false` if it is not.
	 */
	bool is_pressed() 
	{
		return (switch_state == HIGH);
	}


	/** Obtain the time since the last press event happened. Note that this 
	 *  will return a value since the press event even if the switch has been 
	 *  released.
	 *
	 * @return The time in milliseconds since the last press event.
	 */
	unsigned long time_since_pressed()
	{
		return (millis() - last_press);
	}


	/** Obtain the time since the last release event happened. This will return
	 *  a value even if the switch has been pressed. 
	 *
	 * @return The time in milliseconds since the last release event.
	 */
	unsigned long time_since_released()
	{
		return (millis() - last_release);
	}

private:
	// Digital pin configuration
	uint8_t switch_pin;           //!< The digital pin the switch connected to
	uint8_t led_pin;              //!< The digital pin the indicator LED connected to

	// Button state information
	uint8_t switch_state;         //!< The current switch state
	bool in_longpress;            //!< Are we in a long press state?
	unsigned long last_press;     //!< The time in millis since last reset that the last press happened (after debounce)
	unsigned long last_release;   //!< The time in millis since last reset that the last release happened (after debounce)

	// Timing control 
	unsigned long debounce_time;  //!< Time to delay during debounce, in milliseconds.
	unsigned long longpress_time; //!< How long the switch must be held to trigger a 'longpress' event

	// State variables needed to persist data over update()s
	uint8_t last_state;           //!< Previous reading from the switch
	unsigned long last_debounce;  //!< The time at which the last state change occurred during debounce
};

#endif