all:
	mkdir -p build
	sdcc src/main.c -o build/

clean:
	rm -rf build

PORT ?= /dev/ttyUSB0
BAUD ?= 9600

flash: all
	stcgal build/main.ihx -p $(PORT) -b $(BAUD)
