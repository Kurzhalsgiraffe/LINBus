CC=avr-gcc
OBJCOPY=avr-objcopy

DUDE=avrdude.exe
CC = avr-g++


#CFLAGS=-Os -DF_CPU=16000000UL -mmcu=atmega328p
CFLAGS=-DF_CPU=16000000 -O3 -DNDEBUG -mmcu=atmega328p -save-temps -Wall
PORT=/dev/ttyACM0

slave_interrupt.o: slave_interrupt.cpp
	${CC} $(CFLAGS) -c slave_interrupt.cpp -o slave_interrupt.o

objects: slave_interrupt.o

clean:
	del *.o
