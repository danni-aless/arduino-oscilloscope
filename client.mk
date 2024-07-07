CC=gcc
CC_OPTS=-O3 -Wall --std=gnu99
BINS=main_client.elf
OBJS=serial_linux.o

.phony: clean all

all: $(BINS)

%.o:	%.c 
	$(CC) $(CC_OPTS) -c -o $@ $<

%.elf:	%.o $(OBJS)
	$(CC) $(CC_OPTS) -o $@ $< $(OBJS)

clean:
	rm -rf $(OBJS) $(BINS) *~ *.o