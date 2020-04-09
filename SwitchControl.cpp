/** @file
 *  Implementation of the SWitchControl class. This file contains the
 *  implmentation of the class used to control, and detect button presses
 *  from, an illuminated push-button switch.
 *
 * @author Chris Page &lt;chris@starforge.co.uk&gt;
 * @copyright MIT License, 2020 Chris Page
 */
/* The MIT License (MIT)
 *
 * Copyright (c) 2020 Chris Page
 *
 * Permission is hereby granted, free of charge, to any person obtaining a copy
 * of this software and associated documentation files (the "Software"), to deal
 * in the Software without restriction, including without limitation the rights
 * to use, copy, modify, merge, publish, distribute, sublicense, and/or sell
 * copies of the Software, and to permit persons to whom the Software is
 * furnished to do so, subject to the following conditions:
 *
 * The above copyright notice and this permission notice shall be included in
 * all copies or substantial portions of the Software.
 *
 * THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
 * IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
 * FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
 * AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
 * LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
 * OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN
 * THE SOFTWARE.
 */

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
