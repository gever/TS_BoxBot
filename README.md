## Tinkering School Boxbot

This project is part of the **Tinkering School Mars Mission**, and is
designed to help bring high-quality robotics experiences to classrooms everywhere.
https://tinkeringschool.com/mars

For more information, collaboration, or kudos, contact: mars@tinkeringschool.com
If you would like to join the project and help us grow the curriculum, iterate the
robotics platforms, improve and extend the software, then consider joining our discord.

Donations to help us bring this to more people can be made at: https://tinkeringschool.com/donate

This work licensed under a Creative Commons Attribution 4.0 license. That means you can use
it any way you like, including commercially, provided that you attribute it to us, The Institute for Applied Tinkering,
and include a link to https://tinkeringschool.com

https://creativecommons.org/licenses/by/4.0/

NOTE: if you get this installed and running, you should have a new Wifi showing up that is the access point on the boxbot. If you connect to that, then go to http://boxbot.home/blox.html
----------
ESP32 pinout (for our current board)
![ESP32 Pinout](https://www.electronicshub.org/wp-content/uploads/2021/02/ESP32-Pinout-1.jpg)
**Wiring**: 

LEFT motor (aka 2): 
* in1 --> 26
* in2 --> 27
* in3 --> 14
* in4 --> 13

RIGHT motor (aka 1): 
* in1 --> 19
* in2 --> 4
* in3 --> 2
* in4 --> 15

Luminosity Sensor 1: 
* OUT --> 34

Luminosity Sensor 2: 
* OUT --> 39 aka VN

Distance Sensor: 
* Trigger -->  5
* Echo --> 18 

Servo Pins: 
* IN --> 32 
* IN --> 33
* IN --> 25 

Servo Use Notes: 
* Currently have to clarify which pin the servo is attached to in main under setup(). The io pins specified under Servo Pins are allotted to the servos. I don't know how it would work to have mulitple servos, that might require triplicates, one servoSetup and one servoMove for each pin. 
* the 90 degree servos are good at going 90 degress. **this does not work with continuos rotation servos** 
