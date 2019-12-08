#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>

void substr(char *str, char *sub, int start, int end);

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

	int servSock = socket(AF_INET, SOCK_STREAM, 0);
	if(servSock == -1) {
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
		if(connect(servSock, (struct sockaddr *) &address, sizeof(address)) == -1) {
			perror("Error connecting to server");
		} else {
			break;
		}
	}
	
	if(i == 3) {
		printf("Failed to connect, exiting\n");
		return 1;
	}
	
	if(strcmp(sendMessage(servSock, "HELLO"), "HELLO DUMBv0 ready!") == 0) {
		printf("Connected successfully.\n");
	} else {
		printf("There was a server-side error. Closing the connection\n");
	}
	
	printf("You can now start entering the commands\n");
	char command[1024];
	printf("> ");
	scanf("%s", command);
	
	char *boxName = "";
	
	while (1) {
		if (strcmp(command, "help") == 0){
			printf("These are the possible commands:\n\n");
			printf("quit\ncreate\ndelete\nopen\nclose\nnext\nput\n\n");								
		} else if(strcmp(command, "quit") == 0) {
			if(strlen(sendMessage(servSock, "GDBYE")) == 0) {
				printf("Disconnected successfully.\n");
				break;
			} else {
				printf("There was an error disconnecting.\n");
			}
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
					char *response = sendMessage(servSock, createMessage);
					if(strcmp(response, "OK!") == 0) {
						printf("Message box created successfully\n");
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
		} else if(strcmp(command, "delete") == 0) {
			printf("okay, give the name of the message box you want to delete\n");
			
			char *name = (char *) malloc(sizeof(char) * 25);
			printf("delete:> ");
			scanf("%s", name);
			
			char *deleteMessage = (char *) malloc(sizeof(char) * 31);
			strcpy(deleteMessage, "DELBX ");
			strcat(deleteMessage, name);
			
			char *response = sendMessage(servSock, deleteMessage);
			if(strcmp(response, "OK!") == 0) {
				printf("Message box deleted successfully\n");
			} else if(strcmp(response, "ER:NEXST") == 0) {
				printf("ERROR: message box does not exist\n");
			} else if(strcmp(response, "ER:OPEND") == 0) {
				printf("ERROR: message box is currently in use\n");
			} else if(strcmp(response, "ER:NOTMT") == 0) {
				printf("ERROR: message box is not empty\n");
			} else if(strcmp(response, "ER:WHAT?") == 0) {
				printf("ERROR: malformed command\n");
			}
		} else if(strcmp(command, "open") == 0) {
			printf("okay, give the name of the message box you want to open\n");
			
			char *name = (char *) malloc(sizeof(char) * 25);
			printf("open:> ");
			scanf("%s", name);
			
			char *openMessage = (char *) malloc(sizeof(char) * 31);
			strcpy(openMessage, "OPNBX ");
			strcat(openMessage, name);
			
			char *response = sendMessage(servSock, openMessage);
			if(strcmp(response, "OK!") == 0) {
				boxName = name;
				printf("Message box opened successfully\n");
			} else if(strcmp(response, "ER:NEXST") == 0) {
				printf("ERROR: message box does not exist\n");
			} else if(strcmp(response, "ER:OPEND") == 0) {
				printf("ERROR: message box is currently in use\n");
			} else if(strcmp(response, "ER:WHAT?") == 0) {
				printf("ERROR: malformed command\n");
			}
		} else if(strcmp(command, "close") == 0) {
			if(boxName == "") {
				printf("ERROR: you do not currently have a message box open\n");
			} else {
				char *closeMessage = (char *) malloc(sizeof(char) * 31);
				strcpy(closeMessage, "CLSBX ");
				strcat(closeMessage, boxName);
				
				char *response = sendMessage(servSock, closeMessage);
				if(strcmp(response, "OK!") == 0) {
					boxName = "";
					printf("Message box closed successfully\n");
				} else if(strcmp(response, "ER:NOOPN") == 0) {
					printf("ERROR: message box '%s' not currently open\n", boxName);
				} else if(strcmp(response, "ER:WHAT?") == 0) {
					printf("ERROR: malformed command\n");
				}
			}
		} else if(strcmp(command, "next") == 0) {
			char *response = sendMessage(servSock, "NXTMG");
			char *sub = (char *) malloc(6);
			substr(response, sub, 0, 3);
			
			if(strcmp(sub, "OK!") == 0) {
				int delimiter = strchr(response, '!') - response;
				substr(response, sub, 3, delimiter);
				int len = atoi(sub);
				sub = (char *) malloc(len + 1);
				substr(response, sub, delimiter + 1, strlen(response));
				
				printf("%s\n", sub);
			} else if(strcmp(response, "ER:EMPTY") == 0) {
				printf("ERROR: message box is empty\n");
			} else if(strcmp(response, "ER:NOOPN") == 0) {
				printf("ERROR: you do not currently have a message box open\n");			
			} else if(strcmp(response, "ER:WHAT?") == 0) {
				printf("ERROR: malformed command\n");
			}
		} else if(strcmp(command, "put") == 0) {
			printf("okay, enter the message you want to send\n");
			
			char *msg = (char *) malloc(1<<16);
			printf("put:> ");
			scanf("%s", msg);
			
			char *putMessage = (char *) malloc(1<<16 + 11);
			char *len = (char *) malloc(6);
			snprintf(len, 5, "%d", strlen(msg));
			
			strcpy(putMessage, "PUTMG!");
			strcat(putMessage, len);
			strcat(putMessage, "!");
			strcat(putMessage, msg);
			
			char *response = sendMessage(servSock, putMessage);
			char *sub = (char *) malloc(4);
			substr(response, sub, 0, 3);
			
			if(strcmp(sub, "OK!") == 0) {
				printf("Message sent successfully\n");
			} else if(strcmp(response, "ER:NOOPN") == 0) {
				printf("ERROR: you do not currently have a message box open\n");
			} else if(strcmp(response, "ER:WHAT?") == 0) {
				printf("ERROR: malformed command\n");
			}
		} else {
			printf("That is not a command, for a command list enter 'help'.\n");
		}
		
		printf("> ");
		scanf("%s", command);
	}
	
	return 0;
}

void substr(char *str, char *sub, int start, int end) {
    memcpy(sub, &str[start], end - start);
    sub[end - start] = '\0';
}
