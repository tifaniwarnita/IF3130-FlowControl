CC=gcc
CFLAGS = -g 
# uncomment this for SunOS
# LIBS = -lsocket -lnsl

all: transmitter receiver clean-obj

transmitter: src/transmitter.o 
	$(CC) -o bin/transmitter src/transmitter.o -lpthread $(LIBS)

receiver: src/receiver.o
	$(CC) -o bin/receiver src/receiver.o -lpthread $(LIBS)

transmitter.o: src/transmitter.c

receiver.o: src/receiver.c

clean:
	rm -f bin/transmitter bin/receiver src/transmitter.o src/receiver.o

clean-obj:
	rm -f src/transmitter.o src/receiver.o src/queue.o	