#include <ICEClass.h>
//See https://github.com/dadamachines/doppler for doppler info

#include "defines.h"

//Global variables
float breath_at_rest = 0;
int breath_array[breath_array_len];
char prev_note = -1;
bool note_on = 0;
int breath_raw = 0;
int prev_breath = 0;
byte prev_CC = 0;
byte current_CC = 0;
byte octave_transpose = 0;
byte semitone_transpose = 0;
bool menu = 0;

//Custom Includes

#include "hex_leds_FPGA.h"
#include "doppler_func.h"
#include "MIDIUSB.h"

ICEClass ice40;
uint16_t hexmapFont[16] = { 0xF99F,0xF22F,0xF42F,0xF17F,0x1F99,0x7F8F,0xF9F8,0x111F,
                            0x7DBE,0x1F9F,0x9F9F,0xADAC,0xF88F,0xE99E,0xF8EF,0x8E8F };


void setup() { // put your setup code here, to run once:
  ice40.upload(hex_leds_FPGA_bin,sizeof(hex_leds_FPGA_bin)); // Upload BitStream Firmware to FPGA -> see variant.h
  delay(100);
  ice40.initSPI();  // start SPI runtime Link to FPGA

  setupBoard();
  setupBreath();
  Serial.begin(9600);  //MIDI Serial Begin
  USB_PANIC(); //Make sure everything is off on reset
}

void loop() {  // put your main code here, to run repeatedly:

  //To test, read buttons and set lights on 4x4 grid
  byte note_midi;
  note_midi = readButtons();
  
  int breath_filt;
  breath_filt = readBreath();
  Serial.println(breath_filt);
  //Set CC2 to breath value
  //Also add an LED to give visual feedback in the future
  current_CC = min(127,max(0,breath_raw));
  if (abs(current_CC-prev_CC)>CC_threshold){
    USBcontrolChange(0, breath_CC, min(127,max(0,breath_raw))); // Set the value of controller 2 (breath) on channel 0 to breath value
    prev_CC = current_CC;
  }
  
  //Panic code
  //If the user sucks in instead of blowing out
  // if (breath_raw<-10){
  //   Serial.println("PANIC");
  //   USB_PANIC();
  // }


  //Send MIDI message logic.
  //Consider breath pressure, current note, previous note.
  //Currently using a filtered breath value to send note on, with raw (most recent) as velocity
  //In the future consider using first differences to look at the rate of change, rather than specific values
  
  bool retrigger_flag = 0;

  if (breath_filt<breath_threshold){
    note_on = 0;
    USBnoteOff(0, prev_note, min(127,max(0,breath_raw)));//);
    retrigger_flag = 0;
  }

  if (breath_filt>breath_threshold){
    retrigger_flag = 1;
  }
  
  if ((note_midi!=prev_note && note_midi>0 && note_midi <128) || retrigger_flag && !note_on){ //&& breath>threshold
    //turn off previous note
    USBnoteOff(0, prev_note, min(127,max(0,breath_raw)));//);  // Channel 0, middle C, normal velocity
    //MidiUSB.flush(); //Moved into the noteOn function
    note_on = 0;

    //turn on new note
    USBnoteOn(0, note_midi, min(127,max(0,breath_raw)));//min(127,max(0,int(breath*127))));   // Channel 0, middle C, normal velocity
    //MidiUSB.flush();  //Moved into the noteOn function
    note_on = 1;
    //Serial.println(note);
    ice40.sendSPI16(note_midi);
    prev_note = note_midi;
  }
  prev_breath = breath_filt;
  delay(2); //Delay to reduce amount of midi data


}














//----------------OLD CODE FOR TESTING---------------------
  //Single Button Test
//  uint16_t button_b = 0;
//  button_b = digitalRead(0);
//  ice40.sendSPI16(button_b); 
//  delay(50);

//  Number output test
//  for(int i = 0 ; i < 10 ; i++){
//      ice40.sendSPI16(hexmapFont[i] );  
//      delay(800);
//  }

//  Counting test
//  uint16_t val = 0;
//  for(int i = 0 ; i < 65536 ; i++){
//      val = i;
//      ice40.sendSPI16(val);  
//      delay(10);
//  }

//  if(breath_filt < breath_threshold){
//    ice40.sendSPI16(note_midi);
//    USBnoteOn(0, note_midi, 80);
//    delay(500);
//    USBnoteOff(0, note_midi, 80);
//  }else{
//    ice40.sendSPI16(0); 
//    noteOn(0x90, note_midi, 0x00);
//    USBnoteOff(0, note_midi, 80);
//  }
//  delay(2000);
