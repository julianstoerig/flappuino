CFLAGS ?= -Os -DF_CPU=16000000UL -mmcu=atmega328p
LDFLAGS ?= -mmcu=atmega328p
ARDUINO_PORT ?= /dev/ttyUSB0

all: deploy

flappuino.hex: flappuino.elf
	avr-objcopy -O ihex -R .eeprom $^ $@

SRC = $(wildcard *.c)
OBJ = $(SRC:.c=.o)
flappuino.elf: $(OBJ)
	avr-gcc -o $@ $^ $(LDFLAGS)

%.o: %.c %.h
	avr-gcc $(CFLAGS) -c $@ $<

deploy: flappuino.hex
	avrdude -F -V -c arduino -p ATMEGA328p -P $(ARDUINO_PORT) -b 115200 -U flash:w:$<

clean:
	rm *.o *.elf *.hex
