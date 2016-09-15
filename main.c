#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>

#include "bpm_table.h"

#define B 64
#define A 32
#define D 16
#define E 8
#define F 4
#define C 2
#define G 1


uint8_t digits[12] = { 
    (A + B + C + D + E + F),
    (B + C),
    (A + B + G + E + D),
    (A + B + G + C + D),
    (F + G + B + C),
    (A + F + G + C + D),
    (A + F + G + E + D + C),
    (F + A + B + C),
    (A + B + C + D + E + F + G),
    (A + B + C + D + F + G),
    (0),
    (G)
};

#define EMPTY_DIGIT 10

uint8_t display[4];

void setup_interrupts() {
    // timer 1
    TCCR1B &= ~(1 << CS11);
    TCCR1B |= (1 << CS10) | (1 << CS12);
    TIMSK1 |= (1 << TOIE1);
    TCNT1 = 65530; // some high-ish value
    
    // timer 0
    TCCR0A |= (1 << COM0A1) | (1 << COM0A0) | (1 << WGM00);
    TCCR0B |= (1 << CS00);
    TIMSK0 |= (1 << TOIE0);
    OCR0A = 255; // turned off

    sei();
}


void setup() {
    setup_interrupts();
}

// There was some strange transient that caused a need for this on breadboard,
// not sure if needed now
uint16_t current_bpm = 60 + 1;

bool buzzing = false;
uint8_t buzzing_counter = 0;
#define SOUND_DURATION 10
#define SOUND_PWM_VALUE 128

uint8_t current_digit = 0;


ISR(TIMER0_OVF_vect) {
    if (++current_digit == 4) current_digit = 0;

    // turn off pnp-s to avoid ghosting
    PORTD |= 0b00001111;

    PORTC |= 0b00111111;
    PORTC &= ~(display[current_digit] & 0b00111111);

    // One LCD segment is on PORTD (board routing limitations), so 6-th bit is
    // used for it
    if (display[current_digit] & 0b01000000) { 
        PORTD &= ~(1 << PD4);
    } else {
        PORTD |= (1 << PD4);
    }

    PORTD &= ~(1 << current_digit);

    if (buzzing) {
        OCR0A = SOUND_PWM_VALUE;
        buzzing_counter++;
        if (buzzing_counter > SOUND_DURATION) {
            OCR0A = 255;
            buzzing = false;
            buzzing_counter = 0;
        }
    }
}


uint8_t animation_tab[] = {A, G, D, G}; // top, center, bottom, center, ...
uint8_t animation_index = 0;

ISR(TIMER1_OVF_vect) {
    // array is indexed from 0, so shifting here
    TCNT1 = 65536 - bpm[current_bpm-20];
    buzzing = 1;

    animation_index++;
    if (animation_index >= sizeof(animation_tab)/sizeof(uint8_t)) animation_index = 0;
    display[0] = animation_tab[animation_index];
}

void display_number(uint16_t number) { // 3-digit, first digit is used for animation
    int tmp[3];
    tmp[0] = number % 10;
    number /= 10;
    tmp[1] = number % 10;
    number /= 10;
    tmp[2] = number % 10;

    // remove leading zeros
    int i = 2;
    for (i = 2; i > 0; --i) {
        if (tmp[i] == 0) tmp[i] = EMPTY_DIGIT;
        else break;
    }

    display[1] = digits[tmp[2]];
    display[2] = digits[tmp[1]];
    display[3] = digits[tmp[0]];
}

#define MAX_BPM 200
#define MIN_BPM 20

int main(void) {
    setup();

    DDRC = 0b00111111;
    DDRD = 0b01011111;
    DDRB = 0b00000011;

    // buttons
    PORTD |= (1 << PD7);
    PORTB |= (1 << PB2);

    while(1) {
        if (!(PIND & (1 << PD7)) && current_bpm > MIN_BPM) {
            current_bpm--;
        }
        if (!(PINB & (1 << PB2)) && current_bpm < MAX_BPM) {
            current_bpm++;
        }
        display_number(current_bpm);
        _delay_ms(50);
    }
}
