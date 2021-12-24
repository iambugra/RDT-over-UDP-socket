CC=g++ 
DEPS = var.h

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $<

all: server.o client.o
	make server
	make client

server: server.o 
	$(CC) -o server server.o -lpthread

client: client.o
	$(CC) -o client client.o -lpthread

clean:
	rm -rf client server
