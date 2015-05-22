#!/bin/bash

# Start netcat server, create fifo and redirect stdin and stdout

rm fifo
mkfifo fifo

echo Starting joystick control on port 1234...

# Start the program
cat fifo | spicat -i 2>&1 | nc -l 1234 > fifo

echo Exitting..

rm fifo

