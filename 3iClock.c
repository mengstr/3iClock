/*
   ATMEGA48

         +----------+ +----------+
         | PC6/-RES |_| ADC5/PC5 | RTC-SCL
         | PD0/RXD      ADC4/PC4 | RTC-SDA
         | PD1/TXD      ADC3/PC3 | LDR (Brightness)
  Digit1 | PD2/INT0     ADC2/PC2 |
  Digit2 | PD3/INT1     ADC1/PC1 |
  Digit3 | PD4/T0       ADC0/PC0 | BUTTON
         | VCC               GND |
         | GND              AREF |
  Seg-G  | PB6/XTAL1        AVCC |
  Seg-DP | PB7/XTAL2     SCK/PB5 | Seg-F
  Digit4 | PD5/T1       MISO/PB4 | Seg-E
  Digit5 | PD6/AIN0     MOSI/PB3 | Seg-D
  Digit6 | PD7/AIN1      -SS/PB2 | Seg-C
  Seg-A  | PB0/ICP1   PCINT1/PB1 | Seg-B
         +-----------------------+

*/

#define F_CPU 8000000UL		// Internal RC oscillator in ATMEGA48

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <util/delay.h>
#include <util/delay_basic.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/eeprom.h>

#include "twi.h"


// Locations in EEPROM
#define EEPROM_THRESHOLD 	0
#define EEPROM_LEVEL 		1


// Delay macros tuned for 8MHz
#define DLY1MS		_delay_loop_2(2000)
#define DLY10MS		_delay_loop_2(20000)
#define DLY100MS	for (uint8_t dly999=0; dly999<10; dly999++,DLY10MS)

// macros for reading the state of the button
#define ButtonPressed (!(PINC & 0x01))
#define ButtonReleased ((PINC & 0x01))





// Bitmaps for the 7-segment display
const uint8_t charmap[] = {
	0,0,0,0,0,0,0,0,				//  !"#$%&'
	0,0,0,0,0,0,128,0,				// ()*+,-./
	63,6,91,79,102,109,125,7, 		// 01234567
	127,111,0,0,0,0,0,0, 			// 89:;<=>?
	0,119,124,88,94,121,113,61, 	// @ABCDEFG
	116,48,30,117,56,55,84,92,		// HIJKLMNO
	115,107,80,109,120,28,62,126, 	// PQRSTUVW
	118,110,91,0,0,0,0,0			// XYZ[\]^_
};

volatile uint8_t num[6];
volatile long tick=0;
const unsigned char digitmask[]={128,64,32,16,8,4};
volatile unsigned char brightness=0;
volatile uint8_t seg[6];
volatile unsigned char second;
volatile unsigned char minute;
volatile unsigned char hour;

uint8_t dimLevel;
uint8_t brightnessThreshold;







ISR(TIMER0_OVF_vect) {
	static unsigned char digit;
	static unsigned char dim;
	unsigned char dot;

	tick++;

	PORTB=0;
	PORTD=digitmask[digit];
	dot=0;
	if (!dim) PORTB=seg[digit];




	digit++;
	if (digit>5) {
		digit=0;
		dim++;
		if (dim>brightness) dim=0;
	}
}


uint16_t ReadADC(uint8_t channel) {
   ADMUX |= channel;                // Channel selection
   ADCSRA |= _BV(ADSC);               // Start conversion
   while(!bit_is_set(ADCSRA,ADIF));   // Loop until conversion is complete
   ADCSRA |= _BV(ADIF);               // Clear ADIF by writing a 1 (this sets the value to 0)
 
   return(ADC);
}



void GetTime() {
	uint8_t tmp;

    tmp=ReadRTC(0);
	second=(tmp&0x0f)+10*((tmp>>4)&0x07);

    tmp=ReadRTC(1);
	minute=(tmp&0x0f)+10*((tmp>>4)&0x07);

    tmp=ReadRTC(2);
	hour=(tmp&0x0f)+10*((tmp>>4)&0x03);
}




void ShowMsgDelay100ms(char *msg, uint8_t loops) {
	uint8_t i;

	for (i=0; i<6; i++) {
		seg[i]=0;
	}

	for (i=0; (i<6) && (msg[i]>0); i++) {
		seg[i]=charmap[msg[i]-32];
	}

	for (i=0; i<loops; i++) {
		DLY100MS;
	}
}


uint8_t Nybble(uint8_t v) {
	uint8_t t;

	t=16*((int)(v/10)) + v%10;
	return t;
}



void WaitForPress(void) {
	uint16_t i;

	ShowMsgDelay100ms("",5);
	ShowMsgDelay100ms("PRESS",1);
	for (i=0; i<10000; i++) {
		DLY10MS;
		if (ButtonPressed) return;
	}
	return;
}



uint8_t GetValue(uint8_t v, uint8_t vMin, uint8_t vMax) {
	uint8_t i;

	ShowMsgDelay100ms("",5);
	for (;;) {
		seg[5]=charmap[16+(v%10)];
		seg[4]=charmap[16+(v/10)];
		DLY100MS;
		DLY100MS;
		DLY100MS;
		for (i=0; i<30; i++) {
			DLY100MS;
			if (ButtonPressed) break;
		}
		if (i>=30) return v;
		v++;
		if (v>vMax) v=vMin;
	}
	return v;
}





void KeyPressMode() {
	uint8_t i;
	uint8_t v;


	ShowMsgDelay100ms("",1);
	while(ButtonPressed);

	for (i=0; i<30; i++) {
		ShowMsgDelay100ms("SET H", 1);
		if (ButtonPressed) {
			hour=GetValue(hour,0,23);
			WriteRTCByte(2,Nybble(hour));    //HOUR
			break;
		}
	}

	for (i=0; i<30; i++) {
		ShowMsgDelay100ms("SET M", 1);
		if (ButtonPressed) {
			minute=GetValue(minute,0,59);
			WriteRTCByte(1,Nybble(minute));    //MINUTE
			break;
		}
	}

	for (i=0; i<30; i++) {
		ShowMsgDelay100ms("ZERO S", 1);
		if (ButtonPressed) {
			WaitForPress();
			WriteRTCByte(0,0x80);    //START RTC, SECOND=00
			break;
		}
	}


	for (i=0; i<30; i++) {
		ShowMsgDelay100ms("BRIGHT", 1);
		if (ButtonPressed) {
			ShowMsgDelay100ms("BRI.",2);
			for (;;) {
				DLY100MS;
				v=ReadADC(3)/10;
				seg[5]=charmap[16+(v%10)];
				seg[4]=charmap[16+(v/10)];
				if (ButtonPressed) break;
			}
			break;
		}
	}


	for (i=0; i<30; i++) {
		ShowMsgDelay100ms("THRESH", 1);
		if (ButtonPressed) {
			dimLevel=GetValue(brightnessThreshold,0,99);
			eeprom_write_byte((uint8_t *)(uint16_t)EEPROM_THRESHOLD,brightnessThreshold);
			break;
		}
	}

	for (i=0; i<30; i++) {
		ShowMsgDelay100ms("LEVEL", 1);
		if (ButtonPressed) {
			dimLevel=GetValue(dimLevel,0,20);
			eeprom_write_byte((uint8_t *)(uint16_t)EEPROM_LEVEL,dimLevel);
			break;
		}
	}

}





int main() { 
	uint8_t i;
	uint8_t light;

	DDRB=0b11111111;	// Segment drivers PB0..PB7 as output 
	PORTB=0;
	DDRD=0b11111100; 	// Digit drivers PD2..PD7 as output
	PORTD=0;
	DDRC=0b00000000;	// All input on PORTC
	PORTC=0b00000001;	// Pullup on BUTTON only


	brightnessThreshold = eeprom_read_byte((uint8_t *)(uint16_t)EEPROM_THRESHOLD);
	if (brightnessThreshold<1) brightnessThreshold=50;
	if (brightnessThreshold>99) brightnessThreshold=50;
	dimLevel = eeprom_read_byte((uint8_t *)(uint16_t)EEPROM_LEVEL);
	if (dimLevel>16) dimLevel=16;

	for (i=0; i<6; i++) num[i]=0;

	//Enable ADC and set 128 prescale
    ADCSRA = _BV(ADEN) | _BV(ADPS2) | _BV(ADPS1) | _BV(ADPS0); 

	// Set Timer0 prescale rate
	TCCR0B |= _BV(CS01);
	// Enable Timer Overflow Interrupts 
	TIMSK0 |= _BV(TOIE0);
	sei();

	begin();		// Initialize i2C

#ifdef SETTIME
	WriteRTCByte(0,0);       //STOP RTC
	WriteRTCByte(1,0x45);    //MINUTE=09
	WriteRTCByte(2,0x13);    //HOUR=13
	WriteRTCByte(3,0x09);    //DAY=5(SUNDAY) AND VBAT=1
	WriteRTCByte(4,0x03);    //DATE=02
	WriteRTCByte(5,0x06);    //MONTH=06
	WriteRTCByte(6,0x12);    //YEAR=12
	WriteRTCByte(0,0x80);    //START RTC, SECOND=00
#endif

	while (1) {
		if (ButtonPressed) {
			KeyPressMode();
		}

		GetTime();
		seg[5]=charmap[16+(second%10)];
		seg[4]=charmap[16+(second/10)];
		seg[3]=charmap[16+(minute%10)] | 0x80;
		seg[2]=charmap[16+(minute/10)];
		seg[1]=charmap[16+(hour%10)] | 0x80;
		seg[0]=charmap[16+(hour/10)];

		if (second%2==0) {
			light=ReadADC(3)/10;
			if (light>brightnessThreshold) {
				brightness=0;
			} else {
				brightness=dimLevel;
			}
		}
		DLY100MS;
	}
} 








