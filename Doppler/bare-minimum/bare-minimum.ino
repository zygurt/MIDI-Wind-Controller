//See https://github.com/dadamachines/doppler for doppler info

#include "defines.h"

//Global variables
float breath_at_rest = 0;
int breath_array[breath_array_len];
int prev_note = -1;
bool note_on = 0;
int breath_raw = 0;
int prev_breath = 0;
int prev_CC = 0;
int current_CC = 0;


//Custom Includes
#include "MIDIUSB.h"

#include "doppler_func.h"



void setup() { // put your setup code here, to run once:
  setupBoard();
  setupBreath();
  Serial.begin(9600);  //USB MIDI Serial Begin
}

void loop() {  // put your main code here, to run repeatedly:

  //To test, read buttons and set lights on 4x4 grid
  int note_midi;
  note_midi = readButtons();
  Serial.println(note_midi);
  int breath_filt;
  breath_filt = readBreath();
//  Serial.println(breath_filt);
  //Set CC2 to breath value
  //Also add an LED to give visual feedback in the future
  current_CC = min(127,max(0,breath_filt*2));
  if (abs(current_CC-prev_CC)>CC_threshold){
    USBcontrolChange(0, breath_CC, min(127,max(0,breath_filt*2))); // Set the value of controller 2 (breath) on channel 0 to breath value
    prev_CC = current_CC;
  }
  
  //Panic code
  //If the user sucks in instead of blowing out
  if (breath_raw<-10){
    Serial.println("PANIC");
    USB_PANIC();
  }


  //Send MIDI message logic.
  //Consider breath pressure, current note, previous note.
  //Currently using a filtered breath value to send note on, with raw (most recent) as velocity
  //In the future consider using first differences to look at the rate of change, rather than specific values
  
  bool retrigger_flag = 0;

  if (breath_filt<breath_threshold){
    note_on = 0;
    USBnoteOff(0, prev_note, min(127,max(0,breath_raw*2)));//);
    retrigger_flag = 0;
  }

  if (breath_filt>breath_threshold){
    retrigger_flag = 1;
  }
  
  if (note_midi!=prev_note && note_midi!=-1 || retrigger_flag && !note_on){ //&& breath>threshold
    //turn off previous note
    USBnoteOff(0, prev_note, min(127,max(0,breath_raw*2)));//);  // Channel 0, middle C, normal velocity
    //MidiUSB.flush(); //Moved into the noteOn function
    note_on = 0;

    //turn on new note
    USBnoteOn(0, note_midi, min(127,max(0,breath_raw*2)));//min(127,max(0,int(breath*127))));   // Channel 0, middle C, normal velocity
    //MidiUSB.flush();  //Moved into the noteOn function
    note_on = 1;
    //Serial.println(note);
    prev_note = note_midi;
  }
  prev_breath = breath_filt;
  delay(2); //Delay to reduce amount of midi data


}
