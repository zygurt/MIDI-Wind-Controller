#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"
//#include "pico/binary_info.h"


#include "PicoSaxDefines.h"
#include "PicoSaxFunc.h"

//#include "bsp/board.h"
//#include "tusb.h"

// void midi_task(void);

// // Invoked when usb bus is suspended
// // remote_wakeup_en : if host allow us  to perform remote wakeup
// // Within 7ms, device must draw an average of current less than 2.5 mA from bus
// void tud_suspend_cb(bool remote_wakeup_en)
// {
//   (void) remote_wakeup_en;
//   blink_interval_ms = BLINK_SUSPENDED;
// }

// // Invoked when usb bus is resumed
// void tud_resume_cb(void)
// {
//   blink_interval_ms = BLINK_MOUNTED;
// }

int main()
{
    stdio_init_all();
    // tusb_init();
    // PicoSax Setup
    setupBoard();
    // Setup ADC pin for breath input
    adc_init();
    adc_gpio_init(BREATH_GPIO);
    adc_select_input(0);
    setupBreath();
    // Set up our UART
    gpio_set_function(UART_TX_PIN, UART_FUNCSEL_NUM(uart0, 0));
    gpio_set_function(UART_RX_PIN, UART_FUNCSEL_NUM(uart0, 1));
    uart_init(UART_ID, BAUD_RATE);
    

    while (true)
    {
        uint8_t note_midi;
        note_midi = readButtons();
        uint8_t breath_filt;
        breath_filt = readBreath();
        //printf("%d\t",breath_filt);
        // Set CC2 to breath value
        current_CC = breath_raw_midi;
        // if (fabs(current_CC - prev_CC) > CC_threshold){
        //      // Set the value of controller 2 (breath) on channel 0 to breath value
        //     MIDICC(0,breath_CC, breath_raw_midi);
        //     prev_CC = current_CC;
        // }

        //-----------Sending MIDI message if needed----------
        bool retrigger_flag = 0;
        if (breath_filt < breath_threshold && note_on == 1){
            note_on = 0;
            gpio_put(PICO_DEFAULT_LED_PIN, 0);
            MIDInoteOff(1, prev_note, breath_raw_midi);

            // msg[0] = 0x80;                    // Note Off - Channel 1
            // msg[1] = prev_note; // Note Number
            // msg[2] = 0;                       // Velocity
            // tud_midi_n_stream_write(0, 0, msg, 3);

            retrigger_flag = 0;
        }

        if (breath_filt > breath_threshold){
            retrigger_flag = 1;
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
        }

        if ((retrigger_flag && (note_midi != prev_note && note_midi > 0 && note_midi < 128)) || retrigger_flag && !note_on){
            if(note_midi != prev_note){
                //Breath and new note || Breath and same note
                // turn off previous note
                // msg[0] = 0x80;                    // Note Off - Channel 1
                // msg[1] = prev_note; // Note Number
                // msg[2] = 0;                       // Velocity
                // tud_midi_n_stream_write(0, 0, msg, 3);
                MIDInoteOff(1, prev_note, breath_raw_midi);
                note_on = 0;
            }
            // turn on new note
            // msg[0] = 0x90;                    // Note On - Channel 1
            // msg[1] = note_midi;                // Note Number
            // msg[2] = breath_raw_midi;           // Velocity
            // tud_midi_n_stream_write(0, 0, msg, 3);

            MIDInoteOn(1, note_midi, breath_raw_midi);
            note_on = 1;
            //uart_puts(UART_ID, &note_midi);
            if(VERBOSE){
                printf("%d\n",note_midi);
            }
            prev_note = note_midi;
        }
        prev_breath = breath_filt;
        sleep_ms(10); // Delay to reduce amount of midi data
    }
}
