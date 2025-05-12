#include <stdio.h>
#include <math.h>
#include "pico/stdlib.h"
#include "hardware/pio.h"
#include "hardware/uart.h"


#include "PicoSaxDefines.h"
#include "PicoSaxFunc.h"
// #include "blink.pio.h"

// void blink_pin_forever(PIO pio, uint sm, uint offset, uint pin, uint freq) {
//     blink_program_init(pio, sm, offset, pin);
//     pio_sm_set_enabled(pio, sm, true);

//     printf("Blinking pin %d at %d Hz\n", pin, freq);

//     // PIO counter program takes 3 more cycles in total than we pass as
//     // input (wait for n + 1; mov; jmp)
//     pio->txf[sm] = (125000000 / (2 * freq)) - 3;
// }

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart1
#define BAUD_RATE 31250 // MIDI Baud

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 4
#define UART_RX_PIN 5

// Global variables
float breath_at_rest = 0;
float breath_array[breath_array_len];
char prev_note = -1;
bool note_on = 0;
int breath_raw = 0;
int prev_breath = 0;
uint8_t prev_CC = 0;
uint8_t current_CC = 0;
uint8_t octave_transpose = 0;
uint8_t semitone_transpose = 0;
bool menu = 0;

int main()
{
    stdio_init_all();
    // PicoSax Setup
    setupBoard();
    setupBreath();
    // PIO Blinking example
    // PIO pio = pio0;
    // uint offset = pio_add_program(pio, &blink_program);
    // printf("Loaded program at %d\n", offset);

    // #ifdef PICO_DEFAULT_LED_PIN
    // blink_pin_forever(pio, 0, offset, PICO_DEFAULT_LED_PIN, 3);
    // #else
    // blink_pin_forever(pio, 0, offset, 6, 3);
    // #endif
    // For more pio examples see https://github.com/raspberrypi/pico-examples/tree/master/pio

    // Set up our UART
    uart_init(UART_ID, BAUD_RATE);
    // Set the TX and RX pins by using the function select on the GPIO
    // Set datasheet for more information on function select
    gpio_set_function(UART_TX_PIN, GPIO_FUNC_UART);
    gpio_set_function(UART_RX_PIN, GPIO_FUNC_UART);

    // Use some the various UART functions to send out data
    // In a default system, printf will also output via the default UART

    // Send out a string, with CR/LF conversions
    // uart_puts(UART_ID, " Hello, UART!\n");

    // For more examples of UART use see https://github.com/raspberrypi/pico-examples/tree/master/uart

    while (true)
    {
        uint8_t note_midi;
        note_midi = readButtons();

        int breath_filt;
        breath_filt = readBreath();
        printf(breath_filt);
        // Set CC2 to breath value
        // Also add an LED to give visual feedback in the future
        current_CC = min(127, max(0, breath_raw));
        if (abs(current_CC - prev_CC) > CC_threshold){
            // USBcontrolChange(0, breath_CC, min(127, max(0, breath_raw))); // Set the value of controller 2 (breath) on channel 0 to breath value
            prev_CC = current_CC;
        }

        //-----------Sending MIDI message if needed----------
        bool retrigger_flag = 0;

        if (breath_filt < breath_threshold){
            note_on = 0;
            //USBnoteOff(0, prev_note, min(127, max(0, breath_raw))); //);
            retrigger_flag = 0;
        }

        if (breath_filt > breath_threshold){
            retrigger_flag = 1;
        }

        if ((note_midi != prev_note && note_midi > 0 && note_midi < 128) || retrigger_flag && !note_on){ //&& breath>threshold
            // turn off previous note
            //USBnoteOff(0, prev_note, min(127, max(0, breath_raw))); //);  // Channel 0, middle C, normal velocity
            // MidiUSB.flush(); //Moved into the noteOn function
            note_on = 0;

            // turn on new note
            //USBnoteOn(0, note_midi, min(127, max(0, breath_raw))); // min(127,max(0,int(breath*127))));   // Channel 0, middle C, normal velocity
            // MidiUSB.flush();  //Moved into the noteOn function
            note_on = 1;
            printf(note_midi);
            prev_note = note_midi;
        }
        prev_breath = breath_filt;
        sleep_ms(2); // Delay to reduce amount of midi data

        // sleep_ms(1000);
    }
}
