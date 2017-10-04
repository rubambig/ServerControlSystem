# Produces the executable from the .c and .h files
# Runs the server manager to start off

all : Manager Server
Manager: manager.c
	gcc -g -Wall manager.c -o manager.o

Server: server.c
	gcc -g -Wall server.c -o server.o

# Runs the server manager with 2 min and 5 max processes
test: test1

test1:
	./manager.o createServer TestServer 3 5
