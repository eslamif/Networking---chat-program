all: server client 

server: chatserve.cpp
		g++ -Wall -o chatserve chatserve.cpp
client: chatclient.c
		gcc -Wall -o chatclient chatclient.c
clean:
		rm -f chatserve chatclient
