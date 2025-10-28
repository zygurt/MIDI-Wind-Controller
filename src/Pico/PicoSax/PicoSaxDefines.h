//Pin Assignment
// Pin	    Function	    Connection
// 1	    GPIO 0	        UART Tx
// 2	    GPIO 1	        LEDs
// 3	    GND	
// 4	    GPIO 2	        SPI SCK | UART 1 Tx
// 5	    GPIO 3	        SPI Tx | UART 1 Rx
// 6	    GPIO 4	        SPI Rx
// 7	    GPIO 5	        SPI CSn
// 8	    GND	
// 9	    GPIO 6	        I2C SDA
// 10	    GPIO 7	        I2C SCL
// 11	    GPIO 8	        Spr
// 12	    GPIO 9	        Gs
// 13	    GND	
// 14	    GPIO 10	        G
// 15	    GPIO 11	        A
// 16	    GPIO 12	        B
// 17	    GPIO 13	        8va
// 18	    GND	
// 19	    GPIO 14	        Dup
// 20	    GPIO 15	        Bb
// 21	    GPIO 16	        Bvb
// 22	    GPIO 17	        F
// 23	    GND	
// 24	    GPIO 18	        E
// 25	    GPIO 19	        D
// 26	    GPIO 20	        Eb
// 27	    GPIO 21	        C
// 28	    GND	
// 29	    GPIO 22	        Ctrl
// 30	    RUN	
// 31	    GPIO 26 | ADC0	Breath
// 32	    GPIO 27 | ADC1	
// 33	    AGND	
// 34	    GPIO 28 | ADC2	
// 35	    ADC_VREF	
// 36	    3V3	
// 37	    3V3 EN	
// 38	    GND	
// 39	    VSYS	
// 40	    VBUS	        5V (USB)

#define VERBOSE 0
#define BREATH_GPIO 26

#define breath_threshold 20 //Breath on/off threshold
#define breath_scale 0.5 //Breath scaling factor
#define breath_array_len 8
#define NEW_NOTE_TIMER 30000 //uS

#define CC_threshold 1
#define breath_CC 2

// UART defines
// By default the stdout UART is `uart0`, so we will use the second one
#define UART_ID uart0  //uart1
#define BAUD_RATE 31250// 31250 MIDI Baud // 115200 USB BAUD
#define DATA_BITS 8
#define STOP_BITS 1
#define PARITY UART_PARITY_NONE

// Use pins 4 and 5 for UART1
// Pins can be changed, see the GPIO function select table in the datasheet for information on GPIO assignments
#define UART_TX_PIN 0 //4
#define UART_RX_PIN 1 //5

// Global variables
float breath_at_rest = 0;
float breath_array[breath_array_len];
char prev_note = 0;
bool note_on = 0;
uint8_t breath_raw_midi = 0;
int prev_breath = 0;
uint8_t prev_CC = 0;
uint8_t current_CC = 0;
int8_t octave_transpose = 1;
int8_t octave_move = 0;
uint8_t semitone_transpose = 0;
bool menu = 0;