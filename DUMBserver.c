#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <pthread.h>
#include <time.h>
#include "DUMBserver.h"

box *first = NULL;
pthread_mutex_t lock;

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
	
	pthread_mutex_init(&lock, NULL);
	
	int *clientSocket = (int *) malloc(sizeof(int));
	while(1) {
		*clientSocket = accept(socketFD, (struct sockaddr *) &address, &size);
		if(*clientSocket == -1) {
			perror("Accept failed");
			return 1;
		}
		
		pthread_t id = 0;
		pthread_create(&id, NULL, clientHandler, (void *) clientSocket);
	}
	
	pthread_mutex_destroy(&lock);
	
	return 0;
}

void *clientHandler(void *clientSocket) {
	int client = *((int *) clientSocket);
	box *openBox = NULL;

	logMessage(client, "connected");

	char *buffer = (char *) malloc(1<<16);
	char *clientMessage = (char *) malloc(1<<16);
	read(client, buffer, 1024);

	while (strcmp(buffer, "GDBYE") != 0){
		logMessage(client, buffer);
		
		char *msg = (char *) malloc(32);
		if (strcmp(buffer, "HELLO") == 0){
			msg = "HELLO DUMBv0 ready!";
		} else {
			substr(buffer, clientMessage, 0, 5);
			
			if (strcmp(clientMessage, "CREAT") == 0){
				int error=0;
				box* messageBox = (box *) malloc(sizeof(box));
				substr(buffer, clientMessage, 6, strlen(buffer));
				messageBox->name = clientMessage;
				messageBox->messages = NULL;
				
				pthread_mutex_lock(&lock);
				
				box *pointer = NULL;
				for (pointer = first; pointer != NULL; pointer = pointer->next){
				 	if (strcmp(pointer->name, clientMessage) == 0) {
				 		msg = "ER:EXIST";
					 	logMessage(client, msg);
					 	error=1;
						break;
				 	}
				}
		
				if (error == 0) {
			        addMsgBox(&first, messageBox);
       				msg = "OK!";
				}
				
				pthread_mutex_unlock(&lock);
				
				free(messageBox);
			} else if (strcmp(clientMessage, "DELBX") == 0) {
				substr(buffer, clientMessage, 6, strlen(buffer));
				
				msg = "OK!";
				
				pthread_mutex_lock(&lock);
				
				int found = 0;
				box *pointer = NULL;
				for (pointer = first; pointer != NULL; pointer = pointer->next) {
					if(pointer == first) {
						if(strcmp(pointer->name, clientMessage) == 0) {
							found = 1;
							
							if(pointer->open) {
								msg = "ER:OPEND";
								logMessage(client, msg);
							} else {
								first = pointer->next;
								
								// FOR MUTEX TESTING
								memset(pointer, 0, sizeof(box));
								// FOR MUTEX TESTING
																
								free(pointer);
							}
														
							break;
						}
					}
					
					if (pointer->next != NULL && strcmp(pointer->next->name, clientMessage) == 0) {
				 		found = 1;
						
						if(pointer->next->open) {				 	
							msg = "ER:OPEND";
							logMessage(client, msg);
						} else {
						 	free(pointer->next);
						 	pointer->next = pointer->next->next;
						}
				 		
				 		break;
				 	}
				}
				
				pthread_mutex_unlock(&lock);
				
				if(!found) {
					msg = "ER:NEXST";
					logMessage(client, msg);
				}
			} else if(strcmp(clientMessage, "OPNBX") == 0) {
				substr(buffer, clientMessage, 6, strlen(buffer));
				
				msg = "OK!";
				
				if(openBox == NULL) {
					pthread_mutex_lock(&lock);
					
					int found = 0;
					box *toOpen = NULL;
					box *pointer = NULL;
					for (pointer = first; pointer != NULL; pointer = pointer->next) {					
						if(strcmp(pointer->name, clientMessage) == 0) {
	 						found = 1;
	 						
							if(pointer->open) {
								msg = "ER:OPEND";
								logMessage(client, msg);							
							} else {
								toOpen = pointer;
							}
						}
					}
					
					if(toOpen != NULL) {
						toOpen->open = 1;
						openBox = toOpen;
					}
					
					pthread_mutex_unlock(&lock);
					
					if(!found) {
						msg = "ER:NEXST";
						logMessage(client, msg);
					}
				} else {
					msg = "ER:ONLY1";
					logMessage(client, msg);
				}
			} else if(strcmp(clientMessage, "CLSBX") == 0) {
				substr(buffer, clientMessage, 6, strlen(buffer));
			
				msg = "OK!";
			
				if(openBox == NULL) {
					msg = "ER:NOOPN";
					logMessage(client, msg);
				} else {
					openBox->open = 0;
					openBox = NULL;
				}
			} else if(strcmp(clientMessage, "NXTMG") == 0) {
				if(openBox == NULL) {
					msg = "ER:NOOPN";
					logMessage(client, msg);
				} else {
					char *m = getMessage(openBox);
					
					if(m == NULL) {
						msg = "ER:EMPTY";
						logMessage(client, msg);
					} else {
						sprintf(msg, "OK!%d!%s", strlen(m), m);
					}
				}
			} else if(strcmp(clientMessage, "PUTMG") == 0) {
				if(openBox == NULL) {
					msg = "ER:NOOPN";
					logMessage(client, msg);
				} else {
					char *args = (char *) malloc(1<<16);
					
					int len;
					char *m = (char *) malloc(1<<16);
					
					sscanf(buffer, "PUTMG!%d!%[^\n]s", &len, m);
					
					message *Message = (message *) malloc(sizeof(message));
					Message->msg = m;
					
					sprintf(msg, "OK!%d", len);
					addMessage(openBox, Message);
				}
			} else {
				msg = "ER:WHAT?";
				logMessage(client, msg);
			}
		}
		
		write(client, msg, strlen(msg) + 1);
		read(client, buffer, 1024);
	}
	
	// close the currently open box
	if(openBox != NULL) {
		openBox->open = 0;
	}
	
	logMessage(client, buffer);
	logMessage(client, "disconnected");
	close(client);
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

void substr(char *str, char *sub, int start, int end) {
    memcpy(sub, &str[start], end - start);
    sub[end - start] = '\0';
}

void addMsgBox(box **first, box *data) {
	box *temp = (box *) malloc(sizeof(box));
	
	temp->name = (char *) malloc(strlen(data->name) + 1);
	strcpy(temp->name, data->name);

	temp->open = 0;
	
	temp->next = *first;
	*first = temp;
}

void addMessage(box *b, message *Message) {
	message *temp = (message *) malloc(sizeof(message));

	temp->msg = (char *) malloc(strlen(Message->msg) + 1);
	strcpy(temp->msg, Message->msg);

	temp->next = NULL;
	
	if(b->messages == NULL) {
		b->messages = temp;
	} else {
		message *ptr;
		for(ptr = b->messages; ptr->next != NULL; ptr = ptr->next);
		ptr->next = temp;
	}
}

char *getMessage(box *b) {
	if(b->messages == NULL) {
		return NULL;
	} else {
		message *msg = b->messages;
		b->messages = b->messages->next;
		return msg->msg;
	}	
}
