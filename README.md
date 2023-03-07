# Turning ON or OFF your PC using an ESP8266

I needed a way to turn my PC On from outside my house just in case I ever need to access a file.

Firstly, I considered using the WakeOnLan protocol which would be fairly easy to set up by using a Raspberry Pi or doing some port-forwarding in my home router.

I opted for a second option, I wanted to use an ESP8266 microcontroller that I had laying around, I'm using the D1 Mini NodeMCU ESP8266-12F from AZ-Delivery. This way I would also be able to monitor the current state of the PC (OFF, ON, SLEEP).

When you press the power button on your PC's case it shortcircuits both power pins on the motherboard and that's when the PC turns ON. It's the same thing for the reset button, it also shortcircuits both reset pins on the motherboard. Knowing this I needed a way to make this connection by sending a signal from the ESP8266.

I wanted to use a component that would isolate the current coming from the PC from the one coming from the ESP8266.

The first thing that came to mind was using a Relay which I bought. It would activate using two 1.5V batteries in series (meaning 3V) so I thought it would work with the 3.3V that come out of the microcontroller. I plugged it into one of the GPIO pins and it didn't work, it did work on the 3.3V pin. This happened because I didn't take into account the maximum current the GPIO pins can output (12mA) which is not enough to activate the relay.

After this failure I decided to use Optocouplers, these components are typically used in low power applications, and due to their non-mechanical nature they're usually longer lasting than a relay. Searching for one that would fit my requirements I ended up getting the 4N25, which is able to operate with those 12mA coming from the GPIO pins.

### Insert small explanation about how optocuplers work

For the power/reset pins I connect one of the GPIO pins to the input of the 4N25 and the other to GND. At the output I connect both cables that go to the motherboard pins. When I pull the GPIO pin to HIGH and current flows through the optocoupler both ends at the output are connected thus creating a shortcircuit, emulating a button press.

The principle for reading the state of the Power LED is the same but the other way around, the LED pins from the motherboard are the ones activating or deactivating the 4N25 and at the output of the IC I hava the 3.3V pin connected to one of the outputs and an Input pin from the ESP8266 at the other, this way it knows when the LED light is ON or OFF.

# Connections

The pins I used are the following:
* D1 (GPIO5) - 
* D2 (GPIO5) - 
* D5 (GPIO5) - 
* D7 (GPIO5) - 
