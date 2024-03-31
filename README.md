## Tinkering School Boxbot

This project is part of the **Tinkering School Mars Mission**, and is
designed to help bring high-quality robotics experiences to classrooms everywhere.
https://tinkeringschool.com/mars

For more information, collaboration, or kudos, contact: mars@tinkeringschool.com
If you would like to join the project and help us grow the curriculum, iterate the
robotics platforms, improve and extend the software, then consider joining our discord.

Donations to help us bring this to more people can be made at: https://tinkeringschool.com/donate

This work licensed under a Creative Commons Attribution 4.0 license. That means you can use
it any way you like, including commercially, provided that you attribute it to us, The Institute for Applied Tinkering, and include a link to https://tinkeringschool.com

https://creativecommons.org/licenses/by/4.0/

NOTE: if you get this installed and running, you should have a new Wifi showing up that is the access point on the boxbot. If you connect to that, then go to http://192.168.4.1/blox.html
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

Distance Sensor: 
* Trigger -->  5
* Echo --> 18 

Servo Pins: 
* IN --> 32 
* IN --> 33
* IN --> 25 

**Servo/Luminosity Use Notes:** 
If you want to have both servos and luminosity sensors attached you will want to put the servos on the Servo 1 and Servo 3 connectors (D12 and D25 respectively), and the luminosity sensors on Servo 2 and 4 (D32 and D33). This is because GPIO pin 25 cannot be used for input because it is used for the Wifi module built into the chip - *but it can be used as a PWM output to drive a servo.*


**development system setup**
Right now, we have only figured out how to "bake cookies" from within VS Code. It should be possible to 
do all of the development steps from the command line as well - *we just haven't worked on that yet*.
1. download visual studio code (mac and linux seem to talk to the board most reliably) -- this may involve installing some form of git.
2. From VSCode, clone this repository (which should automatically install Platform IO and the ESP32 development tools)
3. Restart VSCode

**baking cookies**
Once VS Code is all settled down and ready to get some work done:
1. in the Platform IO panel, select Project Tasks > esp32dev > Platform > Build Filestystem Image
2. then, Project Tasks > esp32dev > Platform > Upload Filesystem Image
3. then, Project Tasks > esp32dev > General > Upload
