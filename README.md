# TeensyTimingController
Use a Teensy to generate precise periodic pulses on 3 channels (with ~1 µs precision)

Basic Teensy code to set up 3 digital channels with periodic pulse trains. For each channel, you can control the 
period and pulse duration. You can start all channels in sync, or with arbitrary offsets. The code does not use PWM feature,
instead it uses softwatre libraries to control the Teensy's timers. 

This code was written to driving camera timing for the Flyception project, but can easily be generalized for 
other timing-related projects. (For Flyception, there is also a fourth output that is not periodic, rather it is
triggered by Serial input. It can be ignored when adapting this code for other uses.)

My testing showed ~1–3 µs of timing error for: 
- cycle-to-cyle jitter in pulse period, 
- cycle-to-cyle jitter in pulse duration,
- phase offset between two channels with same period started "simultaneously" (i.e., on successive lines of code)

## Setup

1. You need a Teensy, the Ardunio IDE, and the [Teensyduino add-on](https://www.pjrc.com/teensy/td_download.html).
    - (This was tested on a Teensy 3.2, but should work on any Teensy 3.x.)

2. Run the Teensyduino installer. This will allow your Ardunio IDE to program Teensys and will automatically provide access to 
the IntervalTimer library.

3. Open the sketch `TimingControllerFC.ino` in the Arduino IDE.

4. Install the [TeensyDelay library](https://github.com/luni64/TeensyDelay):
    - Download the .zip file and in Arduino select `Sketch->Include Library->Add .ZIP Library...`

5. Compile and upload to your Teensy.

## Hardware

As written, the code uses pins 0–3 for digital outputs. I soldered up a small proto-board to
connect the teensy pins to a ribbon cable that can be split up into four 2-conductor cables to 
carry the output signals to downstream instruments (cameras, flashes, etc.).

![Teensy connection photo](https://github.com/HMS-RIC/TeensyTimingController/blob/master/img/Teensy_for_Flyception_Timing.JPG)
![Teensy connection circuit](https://github.com/HMS-RIC/TeensyTimingController/blob/master/img/TeensyTimingCircuit.jpg)
