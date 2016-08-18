#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <avr/sleep.h>
#include <stdbool.h>

#define DOT 128
#define G 64
#define F 32
#define E 16
#define D 8
#define C 4
#define B 2
#define A 1

char digits[12] = { (A + B + C + D + E + F), (B + C), (A + B + G + E + D), (A + B
		+ G + C + D), (F + G + B + C), (A + F + G + C + D), (A + F + G + E + D
		+ C), (F + A + B + C), (A + B + C + D + E + F + G), (A + B + C + D + F
		+ G), (0), (G) };

uint8_t display[4];


// Interrupts using 16-bit Timer1
void setup_interrupt() {
    // timer 1
    TCCR1B &= ~(1 << CS11);
    TCCR1B |= (1 << CS10) | (1 << CS12);
    TIMSK1 |= (1 << TOIE1);
    
    // timer 0
    DDRD |= (1 << PD6);
    TCCR0A |= (1 << COM0A1) | (1 << COM0A0);
    TCCR0A |= (1 << WGM00);
    TCCR0B |= (1 << CS00);
    TIMSK0 |= (1 << TOIE0);
    OCR0A = 250;

    sei(); // enable global interrupts
}


void setup() {
    setup_interrupt();
}


int buzzing = 0;
int cnt = 0;
int cur_digit = 0;
int pause = 0;
ISR(TIMER0_OVF_vect) {

    if (!pause) {
        cur_digit++;
        if (cur_digit == 4) cur_digit = 0;
        PORTD = ~(1 << cur_digit);

        PORTB = ~(display[cur_digit]);
        PORTC = ~(display[cur_digit] >> 2);
        pause = 1;
    } else {
        PORTD = PORTC = PORTB = 0xff;
        pause = 0;
    }

    if (buzzing) {
        OCR0A = 128;
        cnt++;
        if (cnt > 10) {
            OCR0A = 255;
            buzzing = false;
            cnt = 0;
        }
    }

}

uint16_t bpm[] = { 11719,11161,10654,10190,9766,9375,9015,8681,8371,8082,7813,7561,7324,7102,6894,6697,6511,6335,6168,6010,5860,5717,5580,5451,5327,5208,5095,4987,4883,4783,4688,4596,4507,4422,4340,4261,4185,4112,4041,3973,3906,3842,3780,3720,3662,3606,3551,3498,3447,3397,3348,3301,3255,3211,3167,3125,3084,3044,3005,2967,2930,2894,2858,2824,2790,2757,2725,2694,2663,2634,2604,2576,2548,2520,2493,2467,2442,2416,2392,2368,2344,2321,2298,2276,2254,2232,2211,2191,2170,2150,2131,2112,2093,2074,2056,2038,2021,2003,1986,1970,1953,1937,1921,1906,1890,1875,1860,1846,1831,1817,1803,1789,1776,1762,1749,1736,1723,1711,1698,1686,1674,1662,1651,1639,1628,1617,1605,1595,1584,1573,1563,1552,1542,1532,1522,1512,1503,1493,1484,1474,1465,1456,1447,1438,1429,1421,1412,1404,1395,1387,1379,1371,1363,1355,1347,1339,1332,1324,1317,1309,1302,1295,1288,1281,1274,1267,1260,1253,1247,1240,1234,1227,1221,1215,1208,1202,1196,1190,1184,1178,1172 };

int current_bpm = 60;
ISR(TIMER1_OVF_vect) { // here

    TCNT1 = 65536 - bpm[current_bpm-20];
    buzzing = 1;
}


int main(void) {
    setup();

    DDRC = 0xff;

    DDRD  = 0xff;
    DDRB = DDRC = 0xff;

    display[0] = digits[1];
    display[1] = digits[2];
    display[2] = digits[3];
    display[3] = digits[4];

    char buf[10];
    TCNT1 = 65530;

    while(1) {
    }
}
