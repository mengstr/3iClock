#define F_CPU 8000000UL

#include <stdlib.h>
#include <string.h>
#include <inttypes.h>
#include <math.h>
#include <util/delay.h>
#include <util/delay_basic.h>
#include <avr/io.h>
#include <avr/interrupt.h>
#include <compat/twi.h>

#define TWI_FREQ 100000L
#define RTCADDR 0x6F

#define BUFFER_LENGTH 32
#define TWI_BUFFER_LENGTH 32

#define TWI_READY 0
#define TWI_MRX   1
#define TWI_MTX   2
#define TWI_SRX   3
#define TWI_STX   4


static void (*twi_onSlaveTransmit)(void);
static void (*twi_onSlaveReceive)(uint8_t*, int);

static uint8_t twi_masterBuffer[TWI_BUFFER_LENGTH];
static volatile uint8_t twi_masterBufferIndex;
static uint8_t twi_masterBufferLength;

static uint8_t twi_txBuffer[TWI_BUFFER_LENGTH];
static volatile uint8_t twi_txBufferIndex;
static volatile uint8_t twi_txBufferLength;

static uint8_t twi_rxBuffer[TWI_BUFFER_LENGTH];
static volatile uint8_t twi_rxBufferIndex;

static volatile uint8_t twi_error;



void begin(void);
uint8_t requestFrom(uint8_t address, uint8_t quantity);
void beginTransmission(uint8_t address);
uint8_t endTransmission(void);
void send(uint8_t data);
uint8_t available(void);
void twi_init(void);
void twi_setAddress(uint8_t address);
uint8_t twi_readFrom(uint8_t address, uint8_t* data, uint8_t length);
uint8_t twi_writeTo(uint8_t address, uint8_t* data, uint8_t length, uint8_t wait);
uint8_t twi_transmit(uint8_t* data, uint8_t length);
//void twi_attachSlaveRxEvent( void (*function)(uint8_t*, int) );
//void twi_attachSlaveTxEvent( void (*function)(void) );
void twi_reply(uint8_t ack);
void twi_stop(void);
void twi_releaseBus(void);
uint8_t ReadRTCByte(const uint8_t adr);
void WriteRTCByte(const uint8_t adr, const uint8_t data);
//uint8_t DisplayRTCData(const uint8_t adr, const uint8_t validbits);
uint8_t ReadRTC(const uint8_t adr);
