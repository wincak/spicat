// spicat header file

#include <stdint.h>

void nonblock(int state);
int kbhit(void);

typedef struct {
	uint8_t command;
	uint8_t current;
}tx_struct;

/* SPI */
int openSPI(void);
void SPI_send_byte (uint8_t *tx_byte);
void SPI_send_data(tx_struct tx_send);
