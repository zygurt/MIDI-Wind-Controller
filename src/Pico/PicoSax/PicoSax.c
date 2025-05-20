#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/uart.h"
#include "hardware/gpio.h"
#include "hardware/adc.h"


#include "PicoSaxDefines.h"
#include "PicoSaxFunc.h"


int main()
{
    stdio_init_all();
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
            MIDInoteOff(0, prev_note, breath_raw_midi);
            retrigger_flag = 0;
        }

        if (breath_filt > breath_threshold){
            retrigger_flag = 1;
            gpio_put(PICO_DEFAULT_LED_PIN, 1);
        }

        if ((note_midi != prev_note && note_midi > 0 && note_midi < 128) || retrigger_flag && !note_on){
            // turn off previous note
            MIDInoteOff(0, prev_note, breath_raw_midi);
            note_on = 0;

            // turn on new note
            MIDInoteOn(0, note_midi, breath_raw_midi);
            note_on = 1;
            uart_puts(UART_ID, &note_midi);
            printf("%d\n",note_midi);
            prev_note = note_midi;
        }
        prev_breath = breath_filt;
        sleep_ms(2); // Delay to reduce amount of midi data
    }
}
