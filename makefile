# Produces the executable from the .c and .h files
# Runs the server manager to start off

all : Working Server 

Server: server.c
	gcc -g -Wall server.c -o server.o
	
Working: working_version.c
	gcc -g -Wall working_version.c -o working.o
# Runs the server manager with 2 min and 5 max processes
test: test1

test1:
	./working.o createServer TestServer 3 5
