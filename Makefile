.phony: clean all

all: arduino client

arduino:
	make -f arduino.mk

client:
	make -f client.mk

%.hex:
	make -f arduino.mk $@

clean:
	make -f arduino.mk clean
	make -f client.mk clean