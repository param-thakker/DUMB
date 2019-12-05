#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

int main(int argc, char **argv) {
	if(argc != 2) {
		printf("ERROR: Invalid number of arguments.\nUsage: ./DUMBserve [port]\n");
		return 1;
	}

	int socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if(socketFD == -1) {
		perror("Error creating socket");
		return 1;
	}
	
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_addr.s_addr = INADDR_ANY;
	address.sin_port = htons(atoi(argv[1]));
	
	socklen_t size = sizeof(address);
	
	if(bind(socketFD, (struct sockaddr *) &address, sizeof(address)) == -1) {
		perror("Bind failed");
		return 1;
	}
	
	if(listen(socketFD, 10) == -1) {
		perror("Listen failed");
		return 1;
	}
	
	int clientSocket = accept(socketFD, (struct sockaddr *) &address, &size);
	if(clientSocket == -1) {
		perror("Accept failed");
		return 1;
	}
	
	char buffer[1024];
	read(clientSocket, buffer, 1024);
	
	struct sockaddr_in clientIP;
	socklen_t clientSize = sizeof(clientIP);
	getpeername(clientSocket, (struct sockaddr *) &clientIP, &clientSize);
	
	time_t t = time(NULL);
	printf("%s %s\n", asctime(localtime(&t)), inet_ntoa(clientIP.sin_addr));
	
	char *msg = "HELLO DUMBv0 ready!";
	write(clientSocket, msg, strlen(msg) + 1);
	
	return 0;
}
