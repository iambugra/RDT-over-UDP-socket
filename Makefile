CC=g++ 
DEPS = var.h

%.o: %.cpp $(DEPS)
	$(CC) -c -o $@ $<

all: server.o client.o
	make server
	make client

server: server.o 
	$(CC) -o server server.o

client: client.o
	$(CC) -o client client.o 	

clean:
	rm -rf client server
