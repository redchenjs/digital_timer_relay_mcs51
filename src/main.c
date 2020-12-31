/*
 * main.c
 *
 *  Created on: 2020-12-31 06:26
 *      Author: Jack Chen <redchenjs@live.com>
 */

#include <8051.h>
#include <stdint.h>

#define RELAY   P1_7
#define SEG_DAT P0
#define KEY_DAT P1
#define SEG_SEL P2

// STC89C51RC 12T@12.000MHz
#define FOSC  (12000000UL)
#define T1MS  (65536 - FOSC / 12 / 1000 * 1)
#define T50MS (65536 - FOSC / 12 / 1000 * 50 + 15)

// Common-Anode Display
const uint8_t table[] = {0xc0, 0xf9, 0xa4, 0xb0, 0x99, 0x92, 0x82, 0xf8, 0x80, 0x90};

static uint8_t hour = 0, minute = 0, second = 0;
static uint8_t mode = 0, count_10ms = 0, count_1s = 0;

void tf0_isr(void) __interrupt TF0_VECTOR
{
    TL0 = T50MS & 0xff;
    TH0 = T50MS >> 8;

    if (++count_1s == 20) {
        count_1s = 0;

        if (hour || minute || second) {
            RELAY = 0;

            if (second-- == 0) {
                second = 59;

                if (minute-- == 0) {
                    minute = 59;

                    if (hour-- == 0) {
                        hour = 0;
                    }
                }
            }
        } else {
            RELAY = 1;
        }
    }
}

void tf1_isr(void) __interrupt TF1_VECTOR
{
    static uint8_t digit = 0, key_p = 0, key_n = 0;

    TL1 = T1MS & 0xff;
    TH1 = T1MS >> 8;

    if (++count_10ms == 10) {
        count_10ms = 0;

        key_n = KEY_DAT & 0x07;

        if ((key_p & 0x01) && !(key_n & 0x01)) {
            if (mode == 0) {
                TR0 = 0;

                mode = 6;
                count_1s = 0;
            } else if (--mode == 0) {
                TR0 = 1;
            }
        }

        if ((key_p & 0x02) && !(key_n & 0x02)) {
            switch (mode) {
                case 1:
                    second += 60 + 1;
                    break;
                case 2:
                    second += 60 + 10;
                    break;
                case 3:
                    minute += 60 + 1;
                    break;
                case 4:
                    minute += 60 + 10;
                    break;
                case 5:
                    hour += 100 + 1;
                    break;
                case 6:
                    hour += 100 + 10;
                    break;
                default:
                    break;
            }

            hour %= 100;
            minute %= 60;
            second %= 60;
        }

        if ((key_p & 0x04) && !(key_n & 0x04)) {
            switch (mode) {
                case 1:
                    second += 60 - 1;
                    break;
                case 2:
                    second += 60 - 10;
                    break;
                case 3:
                    minute += 60 - 1;
                    break;
                case 4:
                    minute += 60 - 10;
                    break;
                case 5:
                    hour += 100 - 1;
                    break;
                case 6:
                    hour += 100 - 10;
                    break;
                default:
                    break;
            }

            hour %= 100;
            minute %= 60;
            second %= 60;
        }

        key_p = key_n;
    }

    SEG_SEL = 0xff;

    switch (digit) {
        case 0:
            SEG_DAT = mode && mode != (digit + 1) ? 0xbf : table[second % 10];
            SEG_SEL = 0xfe;
            break;
        case 1:
            SEG_DAT = mode && mode != (digit + 1) ? 0xbf : table[second / 10];
            SEG_SEL = 0xfd;
            break;
        case 2:
            SEG_DAT = mode && mode != (digit + 1) ? 0xbf : table[minute % 10];
            SEG_SEL = 0xfb;
            break;
        case 3:
            SEG_DAT = mode && mode != (digit + 1) ? 0xbf : table[minute / 10];
            SEG_SEL = 0xf7;
            break;
        case 4:
            SEG_DAT = mode && mode != (digit + 1) ? 0xbf : table[hour % 10];
            SEG_SEL = 0xef;
            break;
        case 5:
            SEG_DAT = mode && mode != (digit + 1) ? 0xbf : table[hour / 10];
            SEG_SEL = 0xdf;
            break;
        default:
            break;
    }

    if (++digit == 6) {
        digit = 0;
    }
}

int main(void)
{
    TMOD = 0x11;

    TL0 = T50MS & 0xff;
    TH0 = T50MS >> 8;
    TR0 = 1;
    PT0 = 1;
    ET0 = 1;

    TL1 = T1MS & 0xff;
    TH1 = T1MS >> 8;
    TR1 = 1;
    PT1 = 0;
    ET1 = 1;

    EA = 1;

    while (1);
}
