# AltiUno
Firmware for the 1 pyro output altimeter using an ATMega 85 microcontroller.
An altimeter that fire a charge at apogee and beep the apogee altidude after deploying the parachute. This is really ideal for replacing the motor ejection.
For this one because I wanted to make it smaler I used an ATtiny 85.

The original version uses a BMP085 pressure sensor.                     
<img src="/pictures/AltiUno-bmp085.jpg" width="49%">

The latest version uses a BMP180 pressure sensor, however the code is identical.
<img src="/pictures/AltiUno-bmp180.jpg" width="49%">

An smt board using the same code also exist and can be used in smaller rocket.
<img src="/pictures/altiUno-smt.jpg" width="49%">

# Building the code
You will need to download the Arduino ide from the [Arduino web site](https://www.arduino.cc/).
You need to download the [Attiny support](https://code.google.com/archive/p/arduino-tiny/downloads) and install it to your Arduino environement.
You have to load the Arduino Attiny85 boot loader to your ATtiny85 micro controller. 
Make sure that you download the following [support libraries](https://github.com/bdureau/AltimetersLibs) tinyBMP085, tinyWireM, tinyWireS and copy them to the Arduino library folder. To compile it you need to choose the Attiny 85 and the correct USB port.
You will need to use an AVR programmer and an adapter to program the microcotroller, refer to the documentation.

# Using other pressure sensors
Unfortunatly it is not possible to use a bmp280 sensor, I have ported the library but unfortunatly the attiny 85 has not enought on board memory.
