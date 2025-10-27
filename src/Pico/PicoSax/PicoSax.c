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

        uint8_t note_midi, debounce_midi, test_midi_note;
        note_midi = readButtons();

        //debounce moving of multiple keys
            //test_midi_note = note_midi;
            sleep_ms(10);
            debounce_midi  = readButtons();           
            while(debounce_midi != note_midi){
              note_midi = debounce_midi;
              sleep_ms(10);
              debounce_midi  = readButtons();
            }


        uint8_t breath_filt;
        breath_filt = readBreath();
        //printf("%d\t",breath_filt);
        // Set CC2 to breath value
        current_CC = breath_raw_midi;
        if (fabs(current_CC - prev_CC) > CC_threshold){
             // Set the value of controller 2 (breath) on channel 0 to breath value
            //MIDICC(0,breath_CC, breath_raw_midi);
            msg[0] = 0xB0; //Midi CC channel 1
            msg[1] = breath_CC ;
            msg[2] = current_CC ;
            tud_midi_n_stream_write(0, 0, msg, 3);
            msg[0] = 0xB0; //Midi CC channel 1
            msg[1] = 3 ;
            msg[2] = breath_filt ;
            tud_midi_n_stream_write(0, 0, msg, 3);
            prev_CC = current_CC;
        }

        //-----------Sending MIDI message if needed----------
        bool retrigger_flag = 0;

        //Turning Note Off
        if (breath_filt < breath_threshold && note_on == 1){
            note_on = 0;
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            // MIDInoteOff(1, prev_note, breath_raw_midi);

            msg[0] = 0x80;                    // Note Off - Channel 1
            msg[1] = prev_note; // Note Number
            msg[2] = 0;                       // Velocity
            tud_midi_n_stream_write(0, 0, msg, 3);
            //last_note_time = get_absolute_time();

            retrigger_flag = 0;
        }

        //Check to see if the breath is still above the threshold, incase a note needs to be retriggered
        if (breath_filt > breath_threshold){
            retrigger_flag = 1;
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
        }

        //If the breath is above the threshold AND the note is different to the previous note AND between 0 and 127
        //OR if breath is above the threshold AND the note is off
        //AND it is longer than the minimum time between notes

        

        if ((retrigger_flag && (note_midi != prev_note && note_midi > 0 && note_midi < 128)) || retrigger_flag && !note_on) {
            if(note_midi != prev_note){
                //Breath and new note || Breath and same note
                // turn off previous note
                msg[0] = 0x80;                    // Note Off - Channel 1
                msg[1] = prev_note; // Note Number
                msg[2] = breath_raw_midi;                       // Velocity
                tud_midi_n_stream_write(0, 0, msg, 3);
                // MIDInoteOff(1, prev_note, breath_raw_midi);
                note_on = 0;
            }
            


            // turn on new note
            msg[0] = 0x90;                    // Note On - Channel 1
            msg[1] = note_midi;                // Note Number
            msg[2] = breath_raw_midi;           // Velocity
            tud_midi_n_stream_write(0, 0, msg, 3);
            //last_note_time = get_absolute_time();

            // MIDInoteOn(1, note_midi, breath_raw_midi);
            note_on = 1;
            //uart_puts(UART_ID, &note_midi);
            if(VERBOSE){
                // printf("%d\n",note_midi);
            }
            prev_note = note_midi;
        }
        prev_breath = breath_filt; 
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