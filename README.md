# SmartPhilips2200

The Project can be used to control a Philips coffee machine via MQTT using an ESP8266 module.

A video for explanation can be found here:
https://youtu.be/-lNmjJ2Ej40

The repository is divided into two parts:

**ESP8266:** Code for the microcontroller
-> There is a simple variant without webserver and one with webserver. The variant with web server allows you to configure the WLAN and MQTT settings in your web browser. In the variant without web server, the settings must be made in code.

**ioBroker:** Example implementation with ioBroker.

The wiring within the coffee machine is as shown in the picture:
![Wiring](https://github.com/chris7topher/SmartPhilips2200/blob/master/images/wiring.png)
