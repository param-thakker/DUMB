#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

char *sendMessage(int server, char *msg) {
	write(server, msg, strlen(msg) + 1);
	
	char *buffer = (char *) malloc(256);
	read(server, buffer, 256);
	return buffer;
}

int main(int argc, char **argv) {
	if(argc != 3) {
		printf("ERROR: Invalid number of arguments.\nUsage: ./DUMBclient [address] [port]\n");
		return 1;
	}

	int socketFD = socket(AF_INET, SOCK_STREAM, 0);
	if(socketFD == -1) {
		perror("Error creating socket");
		return 1;
	}
	
	struct sockaddr_in address;
	address.sin_family = AF_INET;
	address.sin_port = htons(atoi(argv[2]));
	
	if(inet_pton(AF_INET, argv[1], &address.sin_addr) < 1) {
		printf("ERROR: Invalid IP address\n");
	}
	
	int i;
	for(i = 0; i < 3; i++) {
		printf("Connecting (%d/3)...\n", i + 1);
		if(connect(socketFD, (struct sockaddr *) &address, sizeof(address)) == -1) {
			perror("Error connecting to server");
		} else {
			break;
		}
	}
	
	if(i == 3) {
		printf("Failed to connect, exiting\n");
		return 1;
	}
	
	if(strcmp(sendMessage(socketFD, "HELLO"), "HELLO DUMBv0 ready!") == 0) {
		printf("Connected successfully.\n");
	} else {
		printf("There was a server-side error. Closing the connection\n");
	}
	
	printf("You can now start entering the commands\n");
	char command[20];
	scanf("%s",command);
	
	while (1) {
		if (strcmp(command, "help") == 0){
			printf("These are the possible commands:\n\n");
			printf("quit\ncreate\ndelete\nopen\nclose\nnext\nput\n\n");								
		} else if(strcmp(command, "create") == 0) {
			printf("okay, give the name of the message box you want to create\n");
			
			char *name = (char *) malloc(sizeof(char) * 25);
			printf("create:> ");
			scanf("%s", name);
			
			char *createMessage = (char *) malloc(sizeof(char) * 31);
			strcpy(createMessage, "CREAT ");
			strcat(createMessage, name);

			if(strlen(name) >= 5 && strlen(name) <= 25) {
				if(tolower(name[0]) >= 'a' && tolower(name[0]) <= 'z') {
					char *response = sendMessage(socketFD, createMessage);
					if(strcmp(response, "OK!") == 0) {
						printf("Message Box Created Successfully\n");
					} else if(strcmp(response, "ER:EXIST") == 0) {
						printf("ERROR: message box already exists\n");
					} else if(strcmp(response, "ER:WHAT?") == 0) {
						printf("ERROR: malformed command\n");
					}
				} else {
					printf("ERROR: message box name must begin with a letter\n");
				}
			} else {
				printf("ERROR: message box name must be between 5-25 characters\n");
			}
		} else if(strcmp(command, "quit") == 0) {
			if(strlen(sendMessage(socketFD, "GDBYE")) == 0) {
				printf("Disconnected successfully.\n");
				break;
			} else {
				printf("There was an error disconnecting.\n");
			}
		}
	
		printf("> ");
		scanf("%s", command);
	}
	
	return 0;
}
