// spicat header file

#include <stdint.h>

void nonblock(int state);
int kbhit(void);

typedef struct {
	uint8_t address;
	uint8_t command;
	uint8_t current;
}tx_struct;

typedef struct {
	uint8_t dutycycle;
	uint8_t current_req;
	uint8_t H_current;
	uint8_t L_current;
	uint8_t trans_temp;
	uint8_t motor_temp;
	uint8_t batt_voltage;
	uint8_t status_byte;
}rx_struct;

/* SPI */
int openSPI(void);
rx_struct SPI_exchange_data(tx_struct tx_data);
/*
void SPI_send_byte (uint8_t *tx_byte);
void SPI_send_data(uint8_t address, tx_struct tx_send);
uint8_t SPI_receive_byte(void);
rx_struct SPI_receive_data(void);
*/
