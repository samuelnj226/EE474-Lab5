# Overview

This is a collection of programs to be used with a Beaglebone mounted on a drive
base with a distance sensor on the front, left, right, and rear of the drive base.
Additionally a BlueSmirf bluetooth module is used to communicate with a user created android app and
the programs.
 
Refer to the report for information about how to wire everything.

After booting up the beaglebone with the correct configuration of peripherals, the user needs to wait for the green LED to turn on. This signifies to the user that the program is ready to be used. Additionally, this enables bluetooth communication between the android app and the BlueSmirf bluetooth module. Once all setup, the user can use the manual or the self-drive mode to maneuver the tank. For manual drive, the user needs to turn the power button on in the app in order to drive the tank. The user can adjust the speed through a slide bar and manually drive in whichever direction using the app. In order to initiate self-drive mode, the user need to press the self drive button on the app and this lets the tank drive itself. Using the four distance sensors, the tank will maneuver forward until the front distance sensor detects anything close. It will then reverse and turn either right or left depending the distance sensor values on either side. The process repeats until the user wants to stop it by pressig the self-driving button to be off 


In order to ge the program to run on boot, a series of steps need to be taken.
Refer to the subsection titled "tank_entry.sh and tank_entry.service" for
instructions on how to set this up.

# tank_entry.sh and tank_entry.service
These files are used to launch the programs on boot. The following
instructions need to be followed before the boot sequence will work and the
system can be used untethered:

1. Send all of the files in the lab5 subdirectory to /root/lab5 on the Beaglebone
2. Run 'make' in the command line and make sure all executables are compiled correctly
3. Move tank_entry.sh to /usr/bin
4. Type 'chmod u+x /usr/bin/tank_entry.sh' into the terminal
5. Move tank_entry.service to /lib/systemd
6. Type 'cd /etc/systemd/system/' and 'ln /lib/systemd/<scriptname>.service <scriptname>.service' into the terminal

All instructions were retreived from http://mybeagleboneblackfindings.blogspot.com/2013/10/running-script-on-beaglebone-black-boot.html

# bt_listener.exe
This program listens for commands sent to the beaglebone from the android app by 
asynchronously reading from a serial port connected to a blutooth module. It then
interprets these commands and sends them on to the appropriate process on the 
beaglebone which can execute it. On/off commands are sent to tank_entry.exe, while
drive/self-drive commands are sent to tank.exe.

# tank_entry.exe
This program listens for an on/off command from bt_listener.exe. If an on command
is received, it runs tank.exe and adc_listener.exe and terminates them if an off
command is received. This is the entry point for the programs that control the movement
of the tank, and is launched on boot alongside bt_listener.exe. It allows both 
adc_listener.exe and tank.exe to be run in parallel when an on command is received.

# tank.exe
This program controls the drive base. In manual drive mode it will maneuver the
vehicle based on the commands it receives from bt_listener.exe. On the other hand,
when it is set to self-drive mode via a command, it will continue to drive forward 
until it senses an object too close in the front, it will then back up and turn to
the direction that has an object that is further away.

# adc_listener.exe
This program is what is used to retreive data from the four ADC pins. Because the
distance sensors are hooked up to the analog pins, it reads distance data and processes
it. It samples the pin at 200 Hz and averages the data every 20 samples. The averaged
data is continually sent to bt_listener.exe and if in self drive mode, it will also
send the information to tank.exe.

# lab5_common.exe
This is a library of functions common to the programs in the system. The library
provides a high-level API for interacting with individual GPIO and PWM pins.

# TankDrive (Android App)
This is an app for Android phones that can be used to control the tank. When
opened, the app will attempt to connect to the tank. After it has connected, the
user can press the power button to make the tank receptive to input. At this
point, the user can send drive commands to it using the directional buttons and
the speed-slider at the bottom of the screen. The user can also enable auto-drive
mode and tilt-driven steering.