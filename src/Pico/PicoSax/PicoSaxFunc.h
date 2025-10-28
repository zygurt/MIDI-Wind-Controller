#include "pico/stdlib.h"

void setupBoard(void){
    //Setup onboard LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    //Setup Instrument buttons
    for (int n = 8; n <= 22; n++){   
        gpio_init(n);
        gpio_set_dir(n, GPIO_IN);
    }
    

}

void setupBreath(void){
    //const float conversion_factor = 3.3f / (1 << 12);
    breath_at_rest = adc_read();

    int n = 0, num_run = 100;
    for (n = 0; n < num_run; n++){
        breath_at_rest = (breath_at_rest + (float)adc_read()) / 2;
    }
    // Final value is in 0-4096 range
}

uint8_t readBreath(void){
    const float conversion_factor = 3.3f / (1 << 12);
    // breath_array
    float breath = 0;
    int n;
    uint16_t breath_sum = 0, breath_filt = 0; //, breath_raw;
    // Read the breath sensor value - 12bit
    breath = (float)adc_read();
    // Remove initial offset and scale to 0-1
    breath = (breath - breath_at_rest) / (4096 - breath_at_rest);

    // Scale the breath input
    if (breath < 0){
        breath = -1 * pow(-1 * breath, breath_scale);
    }else {
        breath = pow(breath, breath_scale);
    }
    breath_raw_midi = fmin(127, fmax(0, (uint8_t)(breath * 127)));

    // breath_array code
    //  Using pointers because I couldn't get normal array indexing to work
    // This should be updated to use a ring buffer

    //  Shift values right through the breath array
    for (n = 0; n < breath_array_len - 1; n++){
        *(breath_array + n) = *(breath_array + (n + 1));
    }
    // Add new breath value
    *(breath_array + breath_array_len - 1) = breath_raw_midi;
    // Average the breath value array
    for (n = 0; n < breath_array_len; n++){
        breath_sum = breath_sum + *(breath_array + n);
    }
    breath_filt = breath_sum / breath_array_len;
    //printf("%d\n",breath_filt);
    return breath_filt;
}

uint16_t readButtons(void){
    const uint8_t read_order[13] = {14, 12, 11, 10, 9, 8, 15, 17, 18, 19, 20, 21, 22}; //Order to Read GPIO
    uint8_t n = 0;
    uint16_t note_buttons = 0, note_midi = 0;
    // Get Buttons value
    for (n = 0; n < 13; n++){
        //This could be updated to gpio_get_all and then a mask and shift
        note_buttons = note_buttons | (gpio_get(read_order[n]) << n); 
    }
    
    //Check to see if going into menu
    if (note_buttons == 4096 && menu == 0){
        // Enter Menu mode
        menu = 1;
        if(VERBOSE){
                printf("Enter Menu\n");
            }
        while (gpio_get(22)); // Hold while Ctrl button is pressed
    }
    //Check to see if leaving menu
    if ((note_buttons == 32 && menu == 1)){
        // Leave Menu mode
        menu = 0;
        if(VERBOSE){
                printf("Exit Menu\n");
            }
        while (gpio_get(8)); // Hold while Spr button is pressed
    }

    if (menu == 1){
        //Menu control
        //The menu might need to change, as the buttons are more about combinations at the moment
        //rather then entering, changing settings and the exiting.
        if (gpio_get(13)){
            note_buttons = 8192;
        }
        if (gpio_get(16)){
            note_buttons = 16384;
        }
        switch (note_buttons){
        case 64:
            // Bb Transpose
            if(VERBOSE){
                printf("Bb Transpose\n");
            }
            octave_transpose = 0;
            semitone_transpose = -2;
            note_midi = 0;
            while (gpio_get(15)); // Hold while button is pressed
            break;
        case 2048:
            // Concert Pitch
            if(VERBOSE){
                printf("C Transpose\n");
            }
            octave_transpose = 0;
            semitone_transpose = 0;
            note_midi = 0;
            while (gpio_get(21)); // Hold while button is pressed
            break;
        case 1024:
            // Eb Transpose
            if(VERBOSE){
                printf("Eb Transpose\n");
            }
            octave_transpose = 0;
            semitone_transpose = 3;
            note_midi = 0;
            while (gpio_get(20)); // Hold while button is pressed
            break;
        // case 8:
        //     // Semitone Up
        //     semitone_transpose++;
        //     note_midi = 0;
        //     while (digitalRead(3))
        //         ; // Hold while button is pressed
        //     break;
        // case 256:
        //     // Semitone Down
        //     semitone_transpose--;
        //     note_midi = 0;
        //     while (digitalRead(8))
        //         ; // Hold while button is pressed
        //     break;
        case 8192:
            // Octave Up
            if(VERBOSE){
                printf("Octave Up\n");
            }
            octave_move++;
            note_midi = 0;
            while (gpio_get(13)); // Hold while button is pressed
            break;
        case 16384:
            // Octave Down
            if(VERBOSE){
                printf("Octave Down\n");
            }
            octave_move--;
            note_midi = 0;
            while (gpio_get(16)); // Hold while button is pressed
            break;
        default:
            note_midi = 0;
            break;
        }
        sleep_ms(100); //Let the menu buttons debounce
    } else {
        //Musical Control
        switch (note_buttons){
        case 0:
            // C#
            note_midi = 49;
            break;
        case 16:
            // C# (G# alternative)
            note_midi = 49;
            break;
        case 4:
            // C
            note_midi = 48;
            break;
        case 20:
            // C (G# alternative)
            note_midi = 48;
            break;
        case 66:
            // C alternate
            note_midi = 48;
            break;
        case 2:
            // B
            note_midi = 47;
            break;
        case 18:
            // B (G# Alternative)
            note_midi = 47;
            break;
        case 70:
            // A#
            note_midi = 46;
            break;
        case 86:
            // A# (G# Alternative)
            note_midi = 46;
            break;
        case 130:
            // A# alternate
            note_midi = 46;
            break;
        case 82:
            // A# (G# Alternative)
            note_midi = 46;
            break;
        case 134:
            //A# another alternative
            note_midi = 46;
            break;
        case 6:
            // A
            note_midi = 45;
            break;
        case 22:
            // A (G# Alternative)
            note_midi = 45;
            break;
        case 30:
            // G#
            note_midi = 44;
            break;
        case 14:
            // G
            note_midi = 43;
            break;
        case 270:
            // F#
            note_midi = 42;
            break;
        case 286:
            // F# (G# Alternative)
            note_midi = 42;
            break;
        case 142:
            // F
            note_midi = 41;
            break;
        case 158:
            // F (G# Alternative)
            note_midi = 41;
            break;
        case 398:
            // E
            note_midi = 40;
            break;
        case 414:
            // E (G# Alternative)
            note_midi = 40;
            break;
        case 1934:
            // D#
            note_midi = 39;
            break;
        case 1950:
            // D# (G# Alternative)
            note_midi = 39;
            break;
        case 1:
            // D Up
            note_midi = 50;
            break;
        case 910:
            // D
            note_midi = 38;
            break;
        case 926:
            // D (G# alternative)
            note_midi = 38;
            break;
        case 2974:
            // C# Low
            note_midi = 37;
            break;
        case 2958:
            // C Low
            note_midi = 36;
            break;
        default:
            note_midi = 0;
            break;
        }

        // Adjust the octave
        if (note_midi > 0){
            uint8_t octave_buttons = 0;
            uint8_t octave_transpose = 0;
            octave_buttons = gpio_get(13)+(gpio_get(16) << 1);

            switch (octave_buttons){
            case 0:
                octave_transpose = 0;
                break;

            case 1:
                octave_transpose = 1;
                break;

            case 2:
                octave_transpose = -1;
                break;

            case 3:
                octave_transpose = 2;
                break;
            
            default:
                octave_transpose = 0;
                break;
            }

            note_midi += ((octave_move+octave_transpose)*12)+semitone_transpose;
        }
    }
    return note_midi;
}

// // First parameter is the event type (0x09 = note on, 0x08 = note off).
// // Second parameter is note-on/note-off, combined with the channel.
// // Channel can be anything between 0-15. Typically reported to the user as 1-16.
// // Third parameter is the note number (48 = middle C).
// // Fourth parameter is the velocity (64 = normal, 127 = fastest).

// void USBnoteOn(byte channel, byte pitch, byte velocity) {
//   midiEventPacket_t noteOn = {0x09, 0x90 | channel, pitch, velocity};
//   MidiUSB.sendMIDI(noteOn);
//   MidiUSB.flush(); //This forces the note to be sent
// }

void MIDInoteOn(uint8_t channel, uint8_t pitch, uint8_t velocity) {
    //Next update should sanitise the msg_type_byte for each input.
    uint8_t command = 0x90;
    uint8_t msg_type_byte;
    //msg_type_byte = (command << 4) | (channel -1);
    uart_putc_raw(UART_ID, (char)command);
    uart_putc_raw(UART_ID, (char)pitch);
    uart_putc_raw(UART_ID, (char)velocity);
}

// void USBnoteOff(byte channel, byte pitch, byte velocity) {
//   midiEventPacket_t noteOff = {0x08, 0x80 | channel, pitch, velocity};
//   MidiUSB.sendMIDI(noteOff);
//   MidiUSB.flush(); //This forces the note to be sent
// }

void MIDInoteOff(uint8_t channel, uint8_t pitch, uint8_t velocity) {
    uint8_t command = 0x80;
    uint8_t msg_type_byte;
    //msg_type_byte = (command << 4) | (channel -1);
    uart_putc_raw(UART_ID, (char)command);
    uart_putc_raw(UART_ID, (char)pitch);
    uart_putc_raw(UART_ID, (char)velocity);
}

// // First parameter is the event type (0x0B = control change).
// // Second parameter is the event type, combined with the channel.
// // Third parameter is the control number number (0-119).
// // Fourth parameter is the control value (0-127).

// void USBcontrolChange(byte channel, byte control, byte value) {
//   midiEventPacket_t event = {0x0B, 0xB0 | channel, control, value};
//   MidiUSB.sendMIDI(event);
//   MidiUSB.flush(); //This forces the note to be sent
// }

void MIDICC(uint8_t channel, uint8_t control, uint8_t value){
    uint8_t command = 0xB;
    uint8_t msg_type_byte;
    msg_type_byte = (command << 4) | (channel -1);
    uart_putc_raw(UART_ID, (char)msg_type_byte);
    uart_putc_raw(UART_ID, (char)control);
    uart_putc_raw(UART_ID, (char)value);
}

// void USB_PANIC(void) {
//   int n = 0, k = 0;
//   for (k = 0; k < 16; k++) {
//     for (n = 0; n < 128; n++) {
//       midiEventPacket_t noteOff = {0x08, 0x80 | k, n, 0};
//       MidiUSB.sendMIDI(noteOff);
//       MidiUSB.flush(); //This forces the note to be sent
//     }
//   }
// }

void MIDIpanic(void){
    // Use the MIDI panic message, rather than sending note off for all of the notes.
    uint8_t msg = 0xFF;
    uart_putc_raw(UART_ID, (char)msg);
}
