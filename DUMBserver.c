#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>

typedef struct _message {
    char *msg;
    struct _message *next;
} message;

typedef struct _box {
    char *name;
    int user;
    int open;
    struct _box *next;
    message *messages;
} box;

char *getIpAddress(int client);
void logMessage(int client, char* msg);
void substr(char* str, char* sub , int start, int len);
void addMsgBox(box **first, box *data);
void removeMsgBox(box **first, box *data);

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

	char *buffer = (char *) malloc(sizeof(char)*1024);
	char *clientMessage = (char *) malloc(sizeof(char)*100);
	box *first = NULL;
	read(clientSocket, buffer, 1024);

	while (strcmp(buffer, "GDBYE") != 0){
		logMessage(clientSocket, buffer);
		
		char *msg;
		if (strcmp(buffer, "HELLO") == 0){
			msg= "HELLO DUMBv0 ready!";
		} else {
			substr(buffer, clientMessage, 0, 5);
			
			if (strcmp(clientMessage, "CREAT") == 0){
				int error=0;
				box* messageBox = (box *) malloc(sizeof(box));
				substr(buffer, clientMessage, 6, strlen(buffer) - 6);
				messageBox->name = clientMessage;
				
				box *pointer = NULL;
				for (pointer = first; pointer != NULL; pointer = pointer->next){
				 	if (strcmp(pointer->name, clientMessage) == 0) {
				 		msg = "ER:EXIST";
					 	logMessage(clientSocket, msg);
					 	error=1;
						break;
				 	}
				}	
		
				if (error == 0) {
			        addMsgBox(&first, messageBox);
       				msg = "OK!";
				}
				
				free(messageBox);
			} else if (strcmp(clientMessage, "DELBX") == 0) {
				substr(buffer, clientMessage, 6, strlen(buffer) - 6);
				
				msg = "OK!";
				
				int found = 0;
				box *pointer = NULL;
				for (pointer = first; pointer != NULL; pointer = pointer->next) {
					if(pointer == first) {
						if(strcmp(pointer->name, clientMessage) == 0) {
							found = 1;
							
							if(pointer->open) {
								msg = "ER:OPEND";
								logMessage(clientSocket, msg);
							} else {
								first = pointer->next;
								free(pointer);							
							}
														
							break;
						}
					}
					
					if (pointer->next != NULL && strcmp(pointer->next->name, clientMessage) == 0) {
				 		found = 1;
						
						if(pointer->next->open) {				 	
							msg = "ER:OPEND";
							logMessage(clientSocket, msg);
						} else {
						 	free(pointer->next);
						 	pointer->next = pointer->next->next;
						}
				 		
				 		break;
				 	}
				}
				
				if(!found) {
					msg = "ER:NEXST";
					logMessage(clientSocket, msg);
				}
			} else if(strcmp(clientMessage, "OPNBX") == 0) {
				substr(buffer, clientMessage, 6, strlen(buffer) - 6);
			
				msg = "OK!";
			
				int found = 0;
				box *toOpen = NULL;
				box *pointer = NULL;
				for (pointer = first; pointer != NULL; pointer = pointer->next) {
					if(pointer->open && pointer->user == 1) {
						found = 1;
						toOpen = NULL;
						msg = "ER:ONLY1";
						logMessage(clientSocket, msg);						
						break;
					}
				
					if(strcmp(pointer->name, clientMessage) == 0) {
 						found = 1;
 						
						if(pointer->open && pointer->user != 1) {
							msg = "ER:OPEND";
							logMessage(clientSocket, msg);							
						} else if(!pointer->open) {
							toOpen = pointer;
						}
					}
				}
				
				if(toOpen != NULL) {
					toOpen->open = 1;
					toOpen->user = 1;				
				}
				
				if(!found) {
					msg = "ER:NEXST";
					logMessage(clientSocket, msg);
				}
			} else if(strcmp(clientMessage, "CLSBX") == 0) {
				substr(buffer, clientMessage, 6, strlen(buffer) - 6);
			
				msg = "OK!";
			
				int found = 0;
				box *pointer = NULL;
				for (pointer = first; pointer != NULL; pointer = pointer->next) {
					if(strcmp(pointer->name, clientMessage) == 0) {
 						found = 1;
 						 
						if(pointer->open && pointer->user == 1) {
							pointer->open = 0;
							pointer->user = 0;
						} else {
							msg = "ER:NOOPN";
							logMessage(clientSocket, msg);							
						}
					}
				}
				
				if(!found) {
					msg = "ER:NOOPN";
					logMessage(clientSocket, msg);
				}			
			} else if(strcmp(clientMessage, "NXTMG") == 0) {
			} else if(strcmp(clientMessage, "PUTMG") == 0) {
			} else {
				msg = "ER:WHAT?";
				logMessage(clientSocket, msg);
			}
		}
		
		write(clientSocket, msg, strlen(msg) + 1);
		read(clientSocket, buffer, 1024);
	}
	
	logMessage(clientSocket, buffer);
	close(clientSocket);
	logMessage(clientSocket, "disconnected");
	
	return 0;
}

char *getIpAddress(int client) {
	struct sockaddr_in address;
	socklen_t size = sizeof(struct sockaddr_in);
	
	getpeername(client, (struct sockaddr *) &address, &size);
	
	return inet_ntoa(address.sin_addr);
}

void logMessage(int client, char* msg) {
	time_t t = time(NULL);
	printf("%s %s %s\n", strtok(ctime(&t), "\n"), getIpAddress(client), msg);
}

void substr(char* str, char* sub , int start, int len){
    memcpy(sub, &str[start], len);
    sub[len] = '\0';
}

void addMsgBox(box **first, box *data) {
	box *temp = (box *) malloc(sizeof(box));
	
	temp->name = (char *) malloc(strlen(data->name) + 1);
	strcpy(temp->name, data->name);

	temp->open = 0;
	temp->user = 0;
	
	temp->next = *first;
	*first = temp;
}

void removeMsgBox(box **first, box *data) {
	
}
