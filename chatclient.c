//Frank Eslami, cs372, Project 1
//A simple chat system that works for one pair of users using the TCP
//protocol.

/*
As suggested in the project instructions, Beej's Guide was heavily referenced to implement the socket programmi    ng code.
http://beej.us/guide/bgnet/
*/

//chatclient.c - this is the client

#include<stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAX_BUFFER 500 // max number of bytes we can get at once

/* ********** FUNCTION DECLARATIONS ********** */
void *get_in_addr(struct sockaddr *sa);


int main(int argc, char *argv[]){
	int sockfd, numbytes;
	char buffer[MAX_BUFFER];
	char message[MAX_BUFFER];
	struct addrinfo hints, *servinfo, *p;
	int rv;
	char s[INET6_ADDRSTRLEN];
	char * server_handle;
	char * server_message;
	int bytes_sent = 0;

	//Ensure user has hostname and port number arguments
	if (argc != 3) {
		fprintf(stderr,"Invalid arguments. Usage: chatclient <hostname> <port>\n");
		exit(1);
	}

	//Prepare socket
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;

	if ((rv = getaddrinfo(argv[1], argv[2], &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	// loop through all the results and connect to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
			perror("client: socket");
			continue;
		}

		if (connect(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("client: connect");
			continue;
		}
		break;
	}

	if (p == NULL) {
		fprintf(stderr, "client: failed to connect\n");
		return 2;
	}

	inet_ntop(p->ai_family, get_in_addr((struct sockaddr *)p->ai_addr),
	s, sizeof s);
	printf("client: connecting to %s\n", s);
	freeaddrinfo(servinfo); // all done with this structure

	//Obtain user's handle
	char client_handle[11] = {0};
	printf("Please enter your user handle (1 word, 10 characters or less): ");
	fgets(client_handle, 11, stdin);
	client_handle[strcspn(client_handle, "\n")] = 0;	//get rid of newline

	if (strlen(client_handle) > 10) {
		printf("You entered too many characters.\n");
		exit(1);
	}
	printf("\nHello %s. Chat session with server began. Say hi:\n", client_handle);	

	while(1) {
		//Prompt client for message to send to server
		while(1) {
			printf("%s> ", client_handle);
			fgets(buffer, MAX_BUFFER - 1, stdin);
			if (strlen(buffer) > 490) {
				printf("Your message may not exceed 490 characters. 10 characters are reserved for your handle. Please try again:\n");
			}
			else 
				break;
		}
		
		//Close connection to server if client enters \quit
		if (strncmp(buffer, "\\quit", 5) == 0) {
		    printf("Closing connection to server...\n");
		    break;
		}

		//Prepend client handle to buffer
		memset(message, 0, sizeof(message));
		strncat(message, client_handle, strlen(client_handle));
		message[strlen(message)] = '>';
		strncat(message, buffer, strlen(buffer) - 1);
//		printf("%s\n", message);

		//Send client's message to server
		if ((bytes_sent = send(sockfd, message, strlen(message), 0)) == -1) {
			perror("Message to server failed");
			exit(1);
		}
		else {
			memset(buffer, 0, sizeof(buffer));
			memset(message, 0, sizeof(message));
		}
	
		//Message from server
		if ((numbytes = recv(sockfd, buffer, MAX_BUFFER-1, 0)) == -1) {
			perror("recv");
			exit(1);
		}
		buffer[numbytes] = '\0';

		//Separate server's handle from its message
		server_handle = strtok(buffer, ">");
		server_message = strtok(NULL, "");

		//Close process if server sends \quit
		if (strncmp(server_message, "\\quit", 5) == 0) {
			printf("%s has closed the chat session.\n", server_handle);
			break;
		}

		//Display server's message
		printf("%s> %s", server_handle, server_message);
	}
	close(sockfd);

	return 0;
}


/* ********** FUNCTION DEFINITIONS ********** */
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa){
	if (sa->sa_family == AF_INET) {
	return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
