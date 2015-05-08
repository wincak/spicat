#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>
#include <termios.h>

#include "functions.h"

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))
#define NB_ENABLE 0
#define NB_DISABLE 1

static const char *device = "/dev/spidev0.0";	// puvodne 1.1
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 50000;
static uint16_t delay;

char ch;
int ret = 0;
int fd;

int main (void) {

	tx_struct tx_data;
	
	uint8_t tx[] = {0x00, 0x0f, 0x00, 0x00, 0x00,
			 0x00, 0x00, 0x00, 0x00, 0x00};
	uint8_t rx[] = {0x00, 0x00, 0x00, 0x00, 0x00,
			0x00, 0x00, 0x00, 0x00, 0x00};

	printf("Hello from raspberry!!\n");


	openSPI();
	
	nonblock(NB_ENABLE);
	
	printf("Start...\n");	

/* main loop */

  	while(1){
  		// read(STDIN_FILENO, &ch, 1);
  		if(kbhit()){
  			ch = fgetc(stdin);
			switch (ch){
				case 'w': tx_data.command = 1; printf("forward\n\r"); break;
				case 's': tx_data.command = 2; printf("reverse\n\r"); break;
				case ' ': tx_data.command = 0; printf("halt   \n\r"); break;
				case '\n': break;
				case 'q': return 0;
				default: tx[0] = 0; printf("\r\n");
			}
		}
		
		//tx_data.command = 1;
		SPI_send_data(tx_data);

		usleep(10000);

  	}

	close(fd);

   	nonblock(NB_DISABLE);

	printf("\nExiting...\n\n");  
	
	return 0;
}


void SPI_send_byte (uint8_t *tx_byte){
	uint8_t rx_byte;

	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx_byte,
		.rx_buf = (unsigned long)rx_byte,
		.len = 1,
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

}

void SPI_send_data(tx_struct tx_send){

	SPI_send_byte(&tx_send.command);
	SPI_send_byte(&tx_send.current);
	
	return;
}

void nonblock(int state)
{
    struct termios ttystate;
    //get the terminal state
    tcgetattr(STDIN_FILENO, &ttystate);

    if (state==NB_ENABLE)
    {
        //turn off canonical mode
        ttystate.c_lflag &= ~ICANON;
        //minimum of number input read.
        ttystate.c_cc[VMIN] = 1;
    }
    else if (state==NB_DISABLE)
    {
        //turn on canonical mode
        ttystate.c_lflag |= ICANON;
    }
    //set the terminal attributes.
    tcsetattr(STDIN_FILENO, TCSANOW, &ttystate);

}

int kbhit()
{
    struct timeval tv;
    fd_set fds;
    tv.tv_sec = 0;  tv.tv_usec = 0;
    FD_ZERO(&fds);
    FD_SET(STDIN_FILENO, &fds); //STDIN_FILENO is 0
    select(STDIN_FILENO+1, &fds, NULL, NULL, &tv);
    return FD_ISSET(STDIN_FILENO, &fds);
}

int openSPI(void){
	fd = open(device, O_RDWR);
	if (fd < 0)
		printf("can't open device");

	/*
	 * spi mode
	 */
	ret = ioctl(fd, SPI_IOC_WR_MODE, &mode);
	if (ret == -1)
		printf("can't set spi mode");

	ret = ioctl(fd, SPI_IOC_RD_MODE, &mode);
	if (ret == -1)
		printf("can't get spi mode");

	/*
	 * bits per word
	 */
	ret = ioctl(fd, SPI_IOC_WR_BITS_PER_WORD, &bits);
	if (ret == -1)
		printf("can't set bits per word");

	ret = ioctl(fd, SPI_IOC_RD_BITS_PER_WORD, &bits);
	if (ret == -1)
		printf("can't get bits per word");

	/*
	 * max speed hz
	 */
	ret = ioctl(fd, SPI_IOC_WR_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't set max speed hz");

	ret = ioctl(fd, SPI_IOC_RD_MAX_SPEED_HZ, &speed);
	if (ret == -1)
		printf("can't get max speed hz");

	printf("spi mode: %d\n", mode);
	printf("bits per word: %d\n", bits);
	printf("max speed: %d Hz (%d KHz)\n", speed, speed/1000);

	return;
}
