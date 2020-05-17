# wifi-kit8-depth
Arduino program for ESP8266 dev board to measure water depth with Milone eTape sensor and publish MQTT.

The "wifi-kit8-depth.ino" code is a simple Arduino program I threw together
to measure the depth of the water in my house's sump using a Milone 
Technologies eTape liquid level sensor.  The resulting measurement is 
displayed on an OLED screen and published to an MQTT broker in a topic 
named "home/sump" as a JSON-encoded value called "depth_inches".

The program was written to run on a Heltek "Wifi Kit 8" ESP8266 development 
board with an integrated I2C OLED display that I bought from Amazon.  (The
board was OK, but I did have to resolder two SMD parts that fell off in 
the course of handling it.  If I had to do it again, I'd probably use 
something from the Adafruit Feather line.)

Heltek WiFi Kit 8: https://heltec.org/project/wifi-kit-8/

Melone Technologies:  https://milonetech.com
Standard eTape Assembly: https://milonetech.com/products/standard-etape-assembly

I purchased a 15-inch eTape assembly with the 0-5V output module so I wouldn't 
have to deal with instrumentation and calibration issues.  The output voltage 
represents liquid height scaled to the length of the tape.  So in this
case, 1V represents a sensed depth of 3 inches, 2.5V represents a depth of 7.5 
inches, etc.  As documented in the data sheet, there is a bit of non-linearity
at the extremes of the scale, but nothing I felt needed to be corrected for.

While the ESP8266 has a single ADC input, it's limited to 3.3 VDC. Rather
than scale the eTape voltage down and reduce resolution, I decided to use
an outboard ADC1015 I2C ADC capable of the full 5V range.  NOTE: The 
sketch from my notebook shows the eTape output connected to input 3 of
the ADC chip, but the program actually uses input 0.

Since the ADC was operating at 5V and many ESP8266 boards run at 3.3V, I
threw in a level shifter board (essentially a couple of MOSFETs with pull-up 
resistors) to make sure WiFi Kit8 and the ADC stayed happy with each other.

The program is pretty straightforward and built on the environment installed
by following the instructions on the Heltec web page using the Arduino board 
manager.  ( https://heltec.org/wifi_kit_install/ ) The dependency libraries 
are also available  via the Arduino IDE using library manager.  Most of the 
code actually comes from the example for the "pubsubclient" library.

I'm using this board in conjunction with a Mosquitto message broker and
Node Red instance running on a Raspberry Pi.  The Node Red flow simply
parses the JSON MQTT messages and displays the depth and 12-hour graph
on the Node Red UI.  I will probably implement this with TLS in the future.
