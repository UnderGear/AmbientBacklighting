

Parts
1 x STEMMA QT / Qwiic JST SH 4-pin to Premium Male Headers Cable (150mm Long)[ID:4209]
1 x Adafruit FT232H Breakout - General Purpose USB to GPIO, SPI, I2C (USB C & Stemma QT)[ID:2264]
1 x Female DC Power adapter - 2.1mm jack to screw terminal block[ID:368]
1 x 5V 10A switching power supply[ID:658]
2 x Adafruit DotStar Digital LED Strip - Black 144 LED/m - One Meter (BLACK) [ID:2241]
2 x 4-pin JST SM Plug + Receptacle Cable Set[ID:578]
1 x Extruded Aluminum Enclosure Box - 100mm x 67mm x 26mm[ID:2229]
 and a random USB-C cable I had lying around

TODO: figure out how to make the housing very chic.

where I learned about SPI, got the idea for the enclosure:
https://swharden.com/blog/2018-06-03-bit-bang-ftdi-usb-to-serial-converters-to-drive-spi-devices/

Adafruit FTDI232H breakout, MPSSE, SPI information
https://ftdichip.com/software-examples/mpsse-projects/libmpsse-spi-examples/
https://ftdichip.com/wp-content/uploads/2020/08/D2XX_Programmers_GuideFT_000071.pdf
https://ftdichip.com/wp-content/uploads/2020/08/AN_178_User-Guide-for-LibMPSSE-SPI-1.pdf
https://learn.adafruit.com/adafruit-ft232h-breakout

Adafruit DotStar LED strip resources
https://learn.adafruit.com/adafruit-dotstar-leds
https://cpldcpu.wordpress.com/2014/11/30/understanding-the-apa102-superled/
https://github.com/adafruit/Adafruit_DotStar/
https://github.com/adafruit/Adafruit_BusIO
		
 wiring
 adapted from https://learn.adafruit.com//assets/63279
 USB connecting breakout and PC
 note: put the I2C switch in the off position if you have the new version of this board
 stemma QT plugged into the socket on the breakout
 stemma QT V+ (red) not connected TODO: clip off?
 stemma QT GND (black) -> strip GND (black)
 stemma QT SDA (blue) -> strip DI (green)
 stemma QT SCL (yellow) -> strip CI (yellow)
 DC 5V (red) -> strip 5V (red)
 DC GND (black) -> strip GND (black)