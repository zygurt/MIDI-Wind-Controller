#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
#include "pico/time.h"
#include "pico/binary_info.h" //USBMIDI


#include "PicoSaxDefines.h"
#include "PicoSaxFunc.h"

#include "bsp/board.h" //USBMIDI
#include "tusb.h" //USBMIDI

enum  {
  BLINK_NOT_MOUNTED = 250,
  BLINK_MOUNTED = 1000,
  BLINK_SUSPENDED = 2500,
};

static uint32_t blink_interval_ms = BLINK_NOT_MOUNTED;
const uint LED_PIN = PICO_DEFAULT_LED_PIN;

void midi_task(void);

absolute_time_t current_time, last_note_time;

int main()
{
    //stdio_init_all();
    
    // tusb_init();
    // PicoSax Setup
    setupBoard();
    // Setup ADC pin for breath input
    adc_init();
    adc_gpio_init(BREATH_GPIO);
    adc_select_input(0);
    setupBreath();
    // Set up our UART
    // gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(uart0, 0));
    // gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(uart0, 1));
    // uart_init(UART_ID, BAUD_RATE);

    board_init(); //USBMIDI
    tusb_init(); //USBMIDI
    

    while (true)
    {
        tud_task(); // tinyusb device task
        uint8_t msg[3];
        uint8_t test_cc_msg[] = {0xB0,74,64};

      // Get the input data
        uint8_t note_midi, debounce_midi, test_midi_note;
        note_midi = readButtons();

        // //debounce moving of multiple keys
        //     //test_midi_note = note_midi;
        //     sleep_ms(10);
        //     debounce_midi  = readButtons();           
        //     while(debounce_midi != note_midi){
        //       note_midi = debounce_midi;
        //       sleep_ms(10);
        //       debounce_midi  = readButtons();
        //     }


        uint8_t breath_filt;
        breath_filt = readBreath();
        //printf("%d\t",breath_filt);
        // Set CC2 to breath value
        current_CC = breath_filt;
        if (fabs(current_CC - prev_CC) > CC_threshold){
            //  // Set the value of controller 2 (breath) on channel 0 to breath value
            // //MIDICC(0,breath_CC, breath_raw_midi);
            msg[0] = 0xB0; //Midi CC channel 1
            msg[1] = breath_CC ;
            msg[2] = breath_filt ;
            tud_midi_n_stream_write(0, 0, msg, 3);
            prev_CC = current_CC;
        }

        //-----------Sending MIDI message if needed----------
        //bool retrigger_flag = 0;

        //Note Logic
        // This used to be multiple individual section.  Reformat to be branching.
        if (breath_filt < breath_threshold ){
          //Turning Note Off
          if ( note_on == 1){
            note_on = 0;
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            // MIDInoteOff(1, prev_note, breath_raw_midi);
            msg[0] = 0x80;                    // Note Off - Channel 1
            msg[1] = prev_note; // Note Number
            msg[2] = breath_filt;                       // Velocity
            tud_midi_n_stream_write(0, 0, msg, 3);
            //last_note_time = get_absolute_time();

            //retrigger_flag = 0;
          }
            
        }else{
          //Breath is above the threshold
          gpio_put(PICO_DEFAULT_LED_PIN, 1);
          if(note_midi != 0){
            //Valid Key Combo
            if(note_on == 0){
              //No current note, so start one
              note_on = 1;
              // turn on new note
              msg[0] = 0x90;                    // Note On - Channel 1
              msg[1] = note_midi;                // Note Number
              msg[2] = breath_raw_midi;           // Velocity
              tud_midi_n_stream_write(0, 0, msg, 3);
              prev_note = note_midi;

            }else{
              //Note is already on
              if (note_midi != prev_note){
                //MIDI CC74 when new note
                // test_cc_msg[2] = 127;
                // tud_midi_n_stream_write(0, 0, test_cc_msg, 3);
                //New note is different to the old note, so there is a changing note
//Make sure that the new note is the final note.  Should be settled after 25ms
                //sleep_ms(50);
                test_midi_note = readButtons();
                //get the current time
                current_time = get_absolute_time();
                //IShould really just keep comparing as time passess, rather than waiting every time
                while(note_midi != test_midi_note || (get_absolute_time()-current_time <NEW_NOTE_TIMER)){
                  //sleep_ms(50);
                  //We want the note to be the same and the time to be greater than the threshold
                  //So store the new note
                  if (test_midi_note != note_midi){
                    //Reset the timer
                    current_time = get_absolute_time();
                    //Save the new note
                    note_midi = test_midi_note;
                    //pulse CC74
                    // test_cc_msg[2] = 0;
                    // tud_midi_n_stream_write(0, 0, test_cc_msg, 3);
                    // test_cc_msg[2] = 127;
                    // tud_midi_n_stream_write(0, 0, test_cc_msg, 3);
                  }
                  
                  //What is the note?
                  test_midi_note = readButtons();
                }
                //Legato note change
                //Turn off CC74 because the note has settled.
                // test_cc_msg[2] = 0;
                // tud_midi_n_stream_write(0, 0, test_cc_msg, 3);
                // turn on new note
                msg[0] = 0x90;                    // Note On - Channel 1
                msg[1] = note_midi;                // Note Number
                msg[2] = breath_raw_midi;           // Velocity
                tud_midi_n_stream_write(0, 0, msg, 3);
                
                //does there need to be a delay?
                sleep_ms(1);

                //turn off old note
                msg[0] = 0x80;                    // Note Off - Channel 1
                msg[1] = prev_note; // Note Number
                msg[2] = breath_raw_midi;                       // Velocity
                tud_midi_n_stream_write(0, 0, msg, 3);

                //Set the new note as previous note
                prev_note = note_midi;
                //No need to set note on, as it is already on.
                note_on = 1; // Just set it anyway.
                if(VERBOSE){
                  // printf("%d\n",note_midi);
                }
              }
            }
          }
        }
        //prev_breath = breath_filt; 
        //sleep_ms(50); // Delay to reduce amount of midi data
    }
}


//--------------------------------------------------------------------+
// Device callbacks
//--------------------------------------------------------------------+

// Invoked when device is mounted
void tud_mount_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}

// Invoked when device is unmounted
void tud_umount_cb(void)
{
  blink_interval_ms = BLINK_NOT_MOUNTED;
}

// Invoked when usb bus is suspended
// remote_wakeup_en : if host allow us  to perform remote wakeup
// Within 7ms, device must draw an average of current less than 2.5 mA from bus
void tud_suspend_cb(bool remote_wakeup_en)
{
  (void) remote_wakeup_en;
  blink_interval_ms = BLINK_SUSPENDED;
}

// Invoked when usb bus is resumed
void tud_resume_cb(void)
{
  blink_interval_ms = BLINK_MOUNTED;
}