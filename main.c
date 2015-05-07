#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <linux/types.h>
#include <linux/spi/spidev.h>

#define ARRAY_SIZE(a) (sizeof(a) / sizeof((a)[0]))

static const char *device = "/dev/spidev0.0";	// puvodne 1.1
static uint8_t mode;
static uint8_t bits = 8;
static uint32_t speed = 5000;
static uint16_t delay;

int main (void) {
	char ch;
	int ret = 0;
	int fd;
	
	uint8_t tx[] = {0xFA};
	uint8_t rx[ARRAY_SIZE(tx)] = {0, };
	
	struct spi_ioc_transfer tr = {
		.tx_buf = (unsigned long)tx,
		.rx_buf = (unsigned long)rx,
		.len = sizeof(tx),
		.delay_usecs = delay,
		.speed_hz = speed,
		.bits_per_word = bits,
	};

  
	printf("Hello from raspberry!!\n");
  
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

//////////////////////////////////////////////////
  	while(read(STDIN_FILENO, &ch, 1) > 0){
		if(ch != '\n'){
			switch (ch){
				case '0': tx[0]=0; break;
				case '1': tx[0]=1; break;
				case '2': tx[0]=2; break;
				case '3': tx[0]=3; break;
				case '4': tx[0]=4; break;
				case '5': tx[0]=5; break;
				case '6': tx[0]=6; break;
				case '7': tx[0]=7; break;
				case '8': tx[0]=8; break;
				case '9': tx[0]=9; break;
				case 'a': tx[0]=10; break;
				case 'b': tx[0]=11; break;
				case 'c': tx[0]=12; break;
				case 'd': tx[0]=13; break;
				case 'e': tx[0]=14; break;
				case 'f': tx[0]=15; break;
				default: tx[0]=(uint8_t)ch;
			}
			
			ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);
		}		
		
		
  	}


	ret = ioctl(fd, SPI_IOC_MESSAGE(1), &tr);

	close(fd);

	return ret;
  
  
  return 0;
}
