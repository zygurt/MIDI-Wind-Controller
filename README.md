# MIDI-Wind-Controller

## A brief overview of the repo

- 3D designs contains dxf and svg files.  Basic design was done in Fusion 360 and then exported.
- Datasheets contains datasheets. Woah.
- src contains source code for doppler and Arduino Leonardo implementations, as well as the board schematic.  Currently midi messages are sent over USB, and shows the midi note number in binary on the LED grid on the doppler board.

## Known issues
- Pressing the low C key (D9) causes the doppler to crash.  Reducing the length of the readButtons() switch case to 7 statements stops the crashing.  The error isn't present on the Arduino Leonardo.
- The octave keys give incorrect values for C#. Top 2 octaves are swapped. Again, this error does not occur for the Arduino Leonardo.

Updates are occasionally posted on [Instagram](https://www.instagram.com/zygurt/)

## Current prototype status:
![Prototype 2](https://raw.githubusercontent.com/zygurt/MIDI-Wind-Controller/main/images/v2.jpg)

## Legacy information
Videos about the initial prototype can be found on [Youtube](https://www.youtube.com/watch?v=xT4aQg9pAro&list=PLZOH4oUjroN9ol0y3P3p1gkSgcQwLBbCZ)
