//Frank Eslami, cs372, Project 1
//A simple chat system that works for one pair of users using the TCP
//protocol. 

/*
As suggested in the project instructions, Beej's Guide was heavily referenced to implement the socket programming code.
http://beej.us/guide/bgnet/
*/

//chatserve - this is the server

#include <cstdio>
#include <cstdlib>
#include <cerrno>
#include <cstring>
#include <csignal>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <iostream>
#include <sstream>
#include <string>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>

using namespace std;

#define BACKLOG 1 // how many pending connections queue will hold
#define SERVER_HANDLE "Frank"

/* ********** FUNCTION DECLARATIONS ********** */
void sigchld_handler(int s);
void *get_in_addr(struct sockaddr *sa);


int main(int argc, char *argv[]) {
	int sockfd, new_fd; // listen on sock_fd, new connection on new_fd
	struct addrinfo hints, *servinfo, *p;
	struct sockaddr_storage their_addr; // connector's address information
	socklen_t sin_size;
	struct sigaction sa;
	int yes=1;
	char s[INET6_ADDRSTRLEN];
	int rv;
	char * client_handle;
	char * client_message;
	char buffer[502] = {0};
	char message[502] = {0};
	char * port_num;

	//Ensure port number is provided by user
	if (argc != 2) {
 	    fprintf(stderr,"Invalid arguments. Usage: chatserve port\n");
 	    exit(1);
 	}
	port_num = argv[1];	//get port number

	//Prepare socket
	memset(&hints, 0, sizeof hints);
	hints.ai_family = AF_UNSPEC;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_flags = AI_PASSIVE; // use my IP

	if ((rv = getaddrinfo(NULL, port_num, &hints, &servinfo)) != 0) {
		fprintf(stderr, "getaddrinfo: %s\n", gai_strerror(rv));
		return 1;
	}
	
	// loop through all the results and bind to the first we can
	for(p = servinfo; p != NULL; p = p->ai_next) {
		if ((sockfd = socket(p->ai_family, p->ai_socktype,
			p->ai_protocol)) == -1) {
		perror("server: socket");
		continue;
		}

		if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes,
			sizeof(int)) == -1) {
			perror("setsockopt");
			exit(1);
		}

		if (bind(sockfd, p->ai_addr, p->ai_addrlen) == -1) {
			close(sockfd);
			perror("server: bind");
			continue;
		}
		break;
	}

	freeaddrinfo(servinfo); // all done with this structure
	if (p == NULL) {
		fprintf(stderr, "server: failed to bind\n");
		exit(1);
	}

	if (listen(sockfd, BACKLOG) == -1) {
		perror("listen");
		exit(1);
	}

	sa.sa_handler = sigchld_handler; // reap all dead processes
	sigemptyset(&sa.sa_mask);
	sa.sa_flags = SA_RESTART;
	if (sigaction(SIGCHLD, &sa, NULL) == -1) {
		perror("sigaction");
		exit(1);
	}

	//Tell user server is listening on specified address	
	char hostname[1024];
	gethostname(hostname, 1024);

	printf("\nchatserve listening on the following address:\n");
	printf("Hostname: %s\n", hostname);
	printf("Port: %s\n", port_num);
	printf("chatserve's user handle: %s\n", SERVER_HANDLE);

	//Listen and accept connection
	int read_size = 0;
	while(1) { // main accept() loop
		sin_size = sizeof their_addr;
		new_fd = accept(sockfd, (struct sockaddr *)&their_addr, &sin_size);
		if (new_fd == -1) {
		perror("accept");
		continue;
		}

		//Echo connection to client
		inet_ntop(their_addr.ss_family,	get_in_addr((struct sockaddr *)&their_addr),
			s, sizeof s);
		printf("server: got connection from %s\n", s);

		//Spawn child process to handle connection
		if (!fork()) { // this is the child process
			close(sockfd); // child doesn't need the listener

			//Read message from client
			printf("\nChat session with client began. Waiting for client...\n");

			while(1) {	//keep connection active until \quit
				if((read_size = recv(new_fd, buffer, 499, 0)) == 1) {
					perror("recv");
					exit(1);
				}
				else if (read_size == 0) {
					printf("Connection to client closed\n");
					printf("\nchatserve listening on the following address:\n");
					printf("Hostname: %s\n", hostname);
					printf("Port: %s\n", port_num);
					printf("chatserve's user handle: %s\n", SERVER_HANDLE);
					break;
				}
				buffer[read_size] = '\0';
			
				//Separate client's handle from its message
				client_handle = strtok(buffer, ">");
				client_message = strtok(NULL, "");
//				printf("Client handle is: %s\n", client_handle);
//				printf("Client message is: %s\n", client_message);

				//Display client's message
				printf("%s> %s\n", client_handle, client_message);
			
				//Obtain response to send to client
				while(1) {
					memset(buffer, 0, sizeof(buffer));
					printf("%s> ", SERVER_HANDLE);
					fgets(buffer, 499, stdin);	
					if (strlen(buffer) > 490) {
					     printf("Your message may not exceed 490 characters. 10 characters are reserved for your handle. Please try again:\n");
					 }
					 else
					     break;
				}

				//Prepend server's handle to server's message to client
				memset(message, 0, sizeof(message));
				strncat(message, SERVER_HANDLE, strlen(SERVER_HANDLE));
				message[strlen(message)] = '>';
				strncat(message, buffer, strlen(buffer));
//				printf("\nServer handle is: %s\n", message);	

				//Send server response to client
				if ((send(new_fd, message, strlen(message), 0)) == -1) {
				    perror("Message to client failed");
				    exit(1);
				}

				//Close connection to client if server enters \quit
				if (strncmp(buffer, "\\quit", 5) == 0) {
				    printf("Closing connection to client...\n");
					break;		
				}

				//Reset buffers
				memset(buffer, 0, sizeof(buffer));
				memset(message, 0, sizeof(message));
			}
			//Displayer server info to user
			printf("\nchatserve listening on the following address:\n");
			printf("Hostname: %s\n", hostname);
			printf("Port: %s\n", port_num);
			printf("chatserve's user handle: %s\n", SERVER_HANDLE);

			close(new_fd);
			exit(0);
		}
		close(new_fd); // parent doesn't need this
	}

	return 0;
}


/* ********** FUNCTION DEFINITIONS ********** */
void sigchld_handler(int s) {
	//waitpid() might overwrite errno, so we save and restore it:
	int saved_errno = errno;
	while(waitpid(-1, NULL, WNOHANG) > 0);
	errno = saved_errno;
}
	
// get sockaddr, IPv4 or IPv6:
void *get_in_addr(struct sockaddr *sa)	{
	if (sa->sa_family == AF_INET) {
		return &(((struct sockaddr_in*)sa)->sin_addr);
	}
	return &(((struct sockaddr_in6*)sa)->sin6_addr);
}
