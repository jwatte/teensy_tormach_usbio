Teensy Tormach USBIO emulator
=============================

This project implements the USB I/O board protocol for the Tormach / PathPilot 
controller. It supports both the input and output directions, mapping them to 
pins on the Teensy. By default, the input pins use the Teensy touch reading functions.
You can easily change this with a #define at the top.

Pick up a Teensy 3.2 board from pjrc, or from Amazon: https://amzn.to/2M3B8oV

Upload this firmware, using the Teensyduino installer on the Arduino IDE: 	https://www.pjrc.com/teensy/td_download.html

By default, this shows up as board 0 but you can change the ID to 1/2/3 if you 
want to support the multiple-boards support of PathPilot.

You also need to install the udev rule (as root, with sudo) in /etc/udev/rules.d, 
found in file 40-teensy.rules

Copyright 2019 Jon Watte; released under MIT license; no warranty given or 
liability accepted.
