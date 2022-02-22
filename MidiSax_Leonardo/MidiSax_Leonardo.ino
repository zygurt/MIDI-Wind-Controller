/*
 * MIDIUSB_test.ino
 *
 * Created: 4/6/2015 10:47:08 AM
 * Author: gurbrinder grewal
 * Modified by Arduino LLC (2015)
 * 
 * MIDI Saxophone by Timothy Roberts
 * 23 October 2020
 * 
 */ 

#include "MIDIUSB.h"


// Function prototypes
void noteOn(byte channel, byte pitch, byte velocity);
void noteOff(byte channel, byte pitch, byte velocity);
void controlChange(byte channel, byte control, byte value);
void PANIC(void);
void pinSetup(void);
int readNote(void);
void breathSetup(void);
int readBreath(void);

// Button define etc.
#define B_Button_Pin 2
#define A_Button_Pin 3
#define G_Button_Pin 4
#define Gs_Button_Pin 5
#define F_Button_Pin 6
#define E_Button_Pin 7
#define D_Button_Pin 8
#define Ds_Button_Pin 9

#define Breath_Pin A0
#define Breath_LED_Pin 11
#define Octave_Pin A1
#define Octave_LED_Pin 10

//#define MIDI_TX_Pin 1
//#define MIDI_RX_Pin 0

#define breath_threshold 15 //Breath on/off threshold
#define breath_scale 0.5 //Breath scaling factor
#define breath_array_len 8

float breath_rest = 0;
int prev_note = -1;
int prev_breath = 0;
bool note_on = 0;
int breath_raw = 0;
int breath_array[breath_array_len];


void setup() {
  Serial.begin(9600);
  pinSetup();
  breathSetup();
}

void loop() {
  int note = -1, breath_filt;
//  float breath = 0;
  breath_filt = readBreath();
  
  
//  Serial.println(breath_filt);
  analogWrite(Breath_LED_Pin,min(max(0,breath_filt<<1),255));
  controlChange(0, 2, min(127,max(0,breath_filt*2))); // Set the value of controller 2 (breath) on channel 0 to breath value
//  controlChange(0, 74, min(int(breath),127)); // Set the value of controller 74 (mpe) on channel 0 to breath value
  //controlChange(0, 7, min(breath,127)); // Set the value of controller 7 (volume) on channel 0 to breath value

  Serial.println(breath_filt);
  
  //Panic code
  if (breath_filt<-10){
    PANIC();
  }
  
  //Read the note value
  note = readNote();
  
  //Change note if needed
  //Need to figure out how to retrigger notes based on breath input

  // I have 2 variables to use here
  // breath_raw is the most recent breath value (converted to int)
  // breath_filt is the filtered breath value
  // Idea is to track the filtered value, and use the raw value to set velocity
  bool retrigger_flag = 0;

  if (breath_filt<breath_threshold){
    note_on = 0;
    noteOff(0, prev_note, min(127,max(0,breath_raw*2)));//);
    retrigger_flag = 0;
  }

  if (breath_filt>breath_threshold){
    retrigger_flag = 1;
  }
  
  if (note!=prev_note && note!=-1 || retrigger_flag && !note_on){ //&& breath>threshold
    //turn off previous note
    noteOff(0, prev_note, min(127,max(0,breath_raw*2)));//);  // Channel 0, middle C, normal velocity
    //MidiUSB.flush(); //Moved into the noteOn function
    note_on = 0;

    //turn on new note
    noteOn(0, note, min(127,max(0,breath_raw*2)));//min(127,max(0,int(breath*127))));   // Channel 0, middle C, normal velocity
    //MidiUSB.flush();  //Moved into the noteOn function
    note_on = 1;
    //Serial.println(note);

    prev_note = note;
  }
  prev_breath = breath_filt;
  delay(2); //Delay to reduce amount of midi data
  
  
}


// -----------------------Function definitions----------------------------------

// First parameter is the event type (0x09 = note on, 0x08 = note off).
// Second parameter is note-on/note-off, combined with the channel.
// Channel can be anything between 0-15. Typically reported to the user as 1-16.
// Third parameter is the note number (48 = middle C).
// Fourth parameter is the velocity (64 = normal, 127 = fastest).

void noteOn(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOn);
  MidiUSB.flush(); //This forces the note to be sent
}

void noteOff(byte channel, byte pitch, byte velocity) {
  midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
  MidiUSB.sendMIDI(noteOff);
  MidiUSB.flush(); //This forces the note to be sent
}

// First parameter is the event type (0x0B = control change).
// Second parameter is the event type, combined with the channel.
// Third parameter is the control number number (0-119).
// Fourth parameter is the control value (0-127).

void controlChange(byte channel, byte control, byte value) {
  midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
  MidiUSB.sendMIDI(event);
  MidiUSB.flush(); //This forces the note to be sent
}

void PANIC(void) {
  int n = 0,k = 0;
  for(k=0;k<16;k++){
    for(n=0;n<128;n++){
      midiEventPacket_t noteOff = {0x08, 0x80 | k, n, 0};
      MidiUSB.sendMIDI(noteOff);
      MidiUSB.flush(); //This forces the note to be sent
    }
  }
}

void pinSetup(void){
  int n = 0;
  for(n=B_Button_Pin;n<Ds_Button_Pin;n++){
    pinMode(n,INPUT);
  }
  pinMode(Breath_LED_Pin, OUTPUT);
  pinMode(Octave_LED_Pin, OUTPUT);
}

void breathSetup(void){
  breath_rest = analogRead(Breath_Pin);
  int n = 0, num_run = 100;
  for(n=0;n<num_run;n++){
    breath_rest = (breath_rest+float(analogRead(Breath_Pin)))/2;
  }
  breath_rest = breath_rest/1024; //Convert to range 0-1
}

int readBreath(void){
  //breath_array
  float breath = 0;
  int n;
  int breath_sum = 0, breath_filt=0;
  //Read the breath sensor value
  breath = (float(analogRead(Breath_Pin))/1024-breath_rest); //Remove atmospheric pressure
  //Scale the breath input
  if(breath<0){
    breath = -1*pow(-1*breath,breath_scale);
  }else{
    breath = pow(breath,breath_scale);
  }
  breath_raw = min(127,max(0,int(breath*127)));

  //breath_array
  //adjust breath array
  // Shift values right through the breath array
  // Using pointers because I couldn't get normal array indexing to work
  for (n=0;n<breath_array_len-1;n++){
    *(breath_array+n) = *(breath_array+(n+1));
  }

//  breath_array[breath_array_len-1] = temp_breath;
  *(breath_array+breath_array_len-1) = breath_raw;
  for (n=0;n<breath_array_len;n++){
    breath_sum = breath_sum + *(breath_array+n);
//    Serial.print(*(breath_array+n));
//    Serial.print(",");
  }
//  Serial.print("\n");
  breath_filt = breath_sum/breath_array_len;
//  Serial.print(temp_breath);
//  Serial.print("\t");
//  Serial.print(breath_sum);
//  Serial.print("\t");
//  Serial.println(breath_filt);
  return breath_filt;
}


int readNote(void){
  //Notes for later
//  Pin 2(B): Port PD1
//  Pin 3(A): Port PD0
//  Pin 4(G): Port PD4
//  Pin 5(Gs): Port PC6
//  Pin 6(F): Port PD7
//  Pin 7(E): Port PE6
//  Pin 8(D): Port PB4
//  Pin 9(Ds): Port PB5
  byte note_byte = 0;
  int note = -1;
  note_byte = note_byte | (PIND & B00000010)>>1; //B
  note_byte = note_byte | (PIND & B00000001)<<1; //A
  note_byte = note_byte | (PIND & B00010000)>>2; //G
  note_byte = note_byte | (PINC & B01000000)>>3; //Gs
  note_byte = note_byte | (PIND & B10000000)>>3; //F
  note_byte = note_byte | (PINE & B01000000)>>1; //E
  note_byte = note_byte | (PINB & B00010000)<<2; //D
  note_byte = note_byte | (PINB & B00100000)<<2; //Ds
//  Serial.print(note_byte);
//Switch case to get note value
  switch (note_byte){
    case 0:
    //C#
      note = 61;
      break;
    case 2:
    //C
      note = 60;
      break;
    case 1:
    //B
      note = 59;
      break;
    case 19:
    //A#
      note = 58;
      break;
    case 3:
    //A
      note = 57;
      break;
    case 15:
    //G#
      note = 56;
      break;
    case 7:
    //G
      note = 55;
      break;
    case 39:
    //F#
      note = 54;
      break;
    case 23:
    //F
      note = 53;
      break;
    case 55:
    //E
      note = 52;
      break;
    case 247:
    //D#
      note = 51;
      break;
    case 119:
    //D
      note = 50;
      break;
    default:
      note = -1;
  }
//  Serial.print("\t");
//  Serial.println(note);

// Adjust the octave
  int octave_read = 0;
  octave_read = analogRead(Octave_Pin);
  if (octave_read>512){
    //top octave
    note += 24;
    analogWrite(Octave_LED_Pin, 255);
  }else if(octave_read>50){
    //middle octave
    note += 12;
    analogWrite(Octave_LED_Pin, 64);
  }else{
    //bottom octave
    //note +=0;
    analogWrite(Octave_LED_Pin, 0);
  }
return note;
}
