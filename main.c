/********************************************************************/
/*	SPICAT															*/
/*  Author: Wincak													*/
/*																	*/
/*	Reads input from stdin and exchanges data over SPI				*/
/*	according to commands. 											*/
/*	Purpose: control of SPI peripherals in regular intervals		*/
/*																	*/
/********************************************************************/

// Usage with netcat (nc)
// HOST:	$ mkfifo fifo
//			$ cat fifo | ./a.out -i 2>&1 | nc -l 1234 -k > fifo
// USER:	$ nc <host_ip> 1234


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
#include "definitions.h"

static const char *device = "/dev/spidev0.0";	// puvodne 1.1
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 50000;
static uint16_t delay;

char ch;
int ret = 0;
int fd;

int main (void) {

	tx_struct tx_left, tx_right;
	rx_struct rx_left, rx_right;
	
	tx_right.address = PIC1;
	tx_left.address = PIC2;
	
	fprintf(stdout,"Hello from raspberry!!\n");

	openSPI();
	
	nonblock(NB_ENABLE);
	
	printf("Start...\n");
	fflush(stdout);	

/* main loop */

  	while(1){
  		if(kbhit()){
  			ch = fgetc(stdin);
			switch (ch){
				case 'w': tx_right.command = motor_CW; printf("forward\n\r"); break;
				case 's': tx_right.command = motor_CCW; printf("reverse\n\r"); break;
				case ' ': tx_right.command = free_run; printf("halt   \n\r"); break;
				case '\n': break;
				case 'q': return 0;
				//default: tx[0] = 0; printf("\r\n");
			}
			fflush(stdout);
		}
		
		SPI_exchange_data(tx_right);

		usleep(10000);

  	}

	close(fd);

   	nonblock(NB_DISABLE);

	printf("\nExiting...\n\n");  
	
	return 0;
}

rx_struct SPI_exchange_data(tx_struct tx_data){
	int ret;
	rx_struct received;
	uint8_t tx[] = {
		0x00, 0x00, 0x00,
		0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00
	};
	uint8_t rx[ARRAY_SIZE(tx)] = {0, };
	
	tx[0] = tx_data.address;
	tx[1] = tx_data.command;
	tx[3] = tx_data.current;
		
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = ARRAY_SIZE(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};	

	// Transfer and receive data
	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
	if (ret < 1){
		printf("can't send spi message\n");
		fflush(stdout);
	}
		
	// Evaluate	
	received.dutycycle = rx[3];
	received.current_req = rx[4];
	received.H_current = rx[5];
	received.L_current = rx[6];
	received.trans_temp = rx[7];
	received.motor_temp = rx[8];
	received.batt_voltage = rx[9];
	received.status_byte = rx[10];

	return received;
}



// Make stdin reading non-blocking
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

// Check for input
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


// Well.... open SPI?
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
