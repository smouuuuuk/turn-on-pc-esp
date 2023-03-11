# Turning ON or OFF your PC using an ESP8266

I needed a way to turn my PC On from outside my house just in case I ever need to access a file.

Firstly, I considered using the WakeOnLan protocol which would be fairly easy to set up by using a Raspberry Pi or doing some port-forwarding in my home router.

I opted for a second option, at the time I was messing around with ESP microcontrollers and wanted to work with one of those. So I searched around for ways to do this and found [noisycarlos's video](https://www.youtube.com/watch?v=rV2b7UrGHbk&t=4s) which had a link to his [website](https://noisycarlos.com/project/how-to-turn-your-computer-on-and-off-remotely/) with code and a schematic for the circuit.

Much of the code is based on his code. I tweaked it a bit so instead of having to send commands to the ESP8266 I could do it through a WebServer hosted by the microcontroller and a few buttons on it. I also added comments to the code and changed a part of the circuit schematic.

I used an ESP8266 microcontroller that I had laying around, the D1 Mini NodeMCU ESP8266-12F from AZ-Delivery.

You first need to know how the pins on the motherboard work whenever you turn it on or reset it by pressing the buttons on your case.

When you press the power button on your PC's case it shortcircuits both power pins on the motherboard and that's when the PC turns ON. It's the same thing for the reset button. Knowing this I needed a way to make this connection by sending a signal from the ESP8266.

In his video, noisycarlos uses some optocouplers to make the shortcircuit between the pins but I decided I wanted to try using some relays, the mechanical part of it and the sound they make when they close the circuit is just that satisfying.

So I bought a few relays. I tested if first at the shop by hooking it up to two 1.5V batteries in series and it worked so I thought it would be fine with the 3.3V that come out of the microcontroller GPIO pins.

I plugged it into one of the GPIO pins and it didn't work, it did work on the 3.3V pin though. This happened because I didn't take into account the maximum current the GPIO pins can output (12mA) which is not enough to activate the relay.

After this I decided to use Optocouplers, these components are typically used in low power applications, and due to their non-mechanical nature they're usually longer lasting than a relay. Searching for one that would fit my requirements I ended up getting a few 4N25, which is able to operate with those 12mA coming from the GPIO pins.

For the power/reset pins I connect one of the GPIO pins to the input of the 4N25 and the other to GND. At the output I connect both cables that go to the motherboard pins. When I pull the GPIO pin to HIGH and current flows through the optocoupler both ends at the output are connected thus creating a shortcircuit, emulating a button press.

The principle for reading the state of the Power LED is the same but the other way around, the LED pins from the motherboard are the ones activating or deactivating the 4N25 and at the output of the IC I hava the 3.3V pin connected to one of the outputs of the IC and an Input pin from the ESP8266 at the other output, this way it knows when the LED light is ON or OFF.

# Connections

The pins I used are the following:
* D1 (GPIO5)  -  Pin used to read if the Power LED is ON or OFF
* D2 (GPIO4)  -  Pin used to turn ON or OFF the Power LED on the PC's case
* D5 (GPIO14) -  Pin used to activate the Reset signal for the PC
* D7 (GPIO13) -  Pin used to activate the Power signal for the PC

A diagram of the circuit can be seen here:

![Final-Schematic](https://user-images.githubusercontent.com/94694675/224386601-a2ec97a3-3545-45e9-a5fb-aa6b1996e648.png)

I've also made a custom PCB and a simple 3D case to enclose everything inside it. Feel free to tweak them however you like!
