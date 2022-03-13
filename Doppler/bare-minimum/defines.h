//This section contains pin assignment defines
//Digital Pins 0-9 are key inputs
  //A3 is breath sensor
  //A5 is Octave buttons [0.33, 1.65, 2.97]V
  //A7 is MIDI out

//If it doesn't compile, use v1.3.0 of the Adafruit SAMD Boards

//For BPS110 pressure sensor: (BPS110-AD0P15-2DG)
//Pin 1 is Vs
//Pin 4 is GND
//Pin 6 is Vout
//The pressure transfer function is
//Ppsi = (Pmax-Pmin)*((Vout-VminComp)/(VmaxComp-VminComp))+Pmin, where:
//Ppsi = Measure Pressure in PSI
//Pmax = Maximum pressure (0.15 psi)
//Pmin = minimum pressure (1 psi)
//VminComp = Minimum Voltage (Usually 0.5V)
//VmaxComp = Maximum Voltage (Usually 4.5V)
//Vout = Output Voltage


#define BREATH_PIN A1
#define OCTAVE_PIN A0

#define breath_threshold 10 //Breath on/off threshold
#define breath_scale 0.5 //Breath scaling factor
#define breath_array_len 8

#define CC_threshold 4
#define breath_CC 2
