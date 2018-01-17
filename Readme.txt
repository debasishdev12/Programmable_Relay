By this project we can make a normal relay as smart programmable relay.

In this project A lcd, four push buttons, a lcd, an arduino and a relay is used.
By pressing push buttons we can set the current for which the relay is tripped.
Here only four push buttons are used to do all operation. It's just like the hand 
watch function in which we have to use two or three buttons for change every digits.

In lcd we can watch what we are doing.

When we set the maximum current value,it is set into the eeprom of arduino.That means
we can use it after power off also.

When the current flow in the circuit is greater than the maximum current we set, the relay 
is cut off and thus no current flows to our device. In this project we can also set the trip time
of the relay.

Hall effect current sensor is used in this project by which we can detect maximum 30amps current.