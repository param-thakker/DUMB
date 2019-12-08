#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

void logMessage(int client, char* msg) {
	time_t t = time(NULL);
	struct sockaddr_in address;
	socklen_t size = sizeof(struct sockaddr_in);
	
	getpeername(client, (struct sockaddr *) &address, &size);
	
	printf("%s %s %s\n", strtok(ctime(&t), "\n"), inet_ntoa(address.sin_addr), msg);
}

void substr(char* str, char* sub , int start, int len);
typedef struct _message {
    char *msg;
    struct _message *next;
}message;
typedef struct _box {
    char *name;
    struct _box *next;
    message *messages;
}box;

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

	
	logMessage(clientSocket, "connected");
	
	char buffer[1024];
	char *clientMessage;

	read(clientSocket, buffer, 1024);

	while (strcmp(buffer, "quit") != 0){
	
	logMessage(clientSocket, buffer);
	struct sockaddr_in clientIP;
	socklen_t clientSize = sizeof(clientIP);
	getpeername(clientSocket, (struct sockaddr *) &clientIP, &clientSize);
	
	time_t t = time(NULL);
	printf("%s %s\n", asctime(localtime(&t)), inet_ntoa(clientIP.sin_addr));
	char *msg;
	if (strcmp(buffer, "HELLO") == 0){
	 msg= "HELLO DUMBv0 ready!";
	}
	else if (strcmp(buffer, "GDBYE") == 0){
	msg="";
	}
	
	if (strcmp(substr(buffer, clientMessage, 0, 4), "CREAT")==0){
		struct box messageBox=(struct box*)malloc(sizeof(struct box));
		messageBox.name=substr(buffer, clientMessage;
		msg="OK!";
		
		
	}
	else if (strcmp(buffer, strcat("CREAT ", name) == 0){
		
	}
	write(clientSocket, msg, strlen(msg) + 1);
	read(clientSocket, buffer, 1024);
	}
	
	while(1) {
		char buffer[1024];
		read(clientSocket, buffer, 1024);
		
		if(strcmp(buffer, "GDBYE") == 0) {
			logMessage(clientSocket, buffer);
			close(clientSocket);
			logMessage(clientSocket, "disconnected");
			
			break;
		}
	}
	
	return 0;
}
void substr(char* str, char* sub , int start, int len){
    memcpy(sub, &str[start], len);
    sub[len] = '\0';
}
