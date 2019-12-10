typedef struct _message {
    char *msg;
    struct _message *next;
} message;

typedef struct _box {
    char *name;
    int open;
    struct _box *next;
    message *messages;
} box;

void *clientHandler(void *clientSocket);
char *getIpAddress(int client);
void logMessage(int client, char* msg);
void substr(char *str, char *sub, int start, int end);
void addMsgBox(box **first, box *data);
void removeMsgBox(box **first, box *data);
void addMessage(box *b, message *Message);
char *getMessage(box *b);
