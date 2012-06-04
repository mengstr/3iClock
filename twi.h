#define TWI_FREQ 100000L

#define BUFFER_LENGTH 32
#define TWI_BUFFER_LENGTH 32

#define TWI_READY 0
#define TWI_MRX   1
#define TWI_MTX   2
#define TWI_SRX   3
#define TWI_STX   4





void begin(void);
uint8_t requestFrom(uint8_t address, uint8_t quantity);
void beginTransmission(uint8_t address);
uint8_t endTransmission(void);
void send(uint8_t data);
uint8_t available(void);
uint8_t receive(void);

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
//uint8_t ReadRTCByte(const uint8_t adr);
//void WriteRTCByte(const uint8_t adr, const uint8_t data);
