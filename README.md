# spicat
Use RaspPi's SPI port from command line just like UART

I need to transmit data between raspberry and PIC18 over SPI, just like it is 
possible with UART. This little software just reads stdin, sends it over SPI and listens to response
in the same way. It should be possible to use it with netcat server (thus the name)

currently work in progress. obviously!
