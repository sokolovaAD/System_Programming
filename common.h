#ifndef COMMON_H 
#define COMMON_H
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <limits.h>
#include <stdbool.h>

#define MAX_DATA_SIZE 3000

#define HANDSHAKE "hello"

enum command {
    EOSIG = 0,
    CD,
    LS,
    CAT,
    UPLOAD,
    DOWNLOAD,
    EXIT,
    HELLO,
    ERROR,
    UPDATE
};

struct message {
    enum command command;
    char path[PATH_MAX];
    unsigned long long size;
    char* data;
};

void sendMsg(struct message* msg, int sd);
struct message* receiveMsg(int sd);
struct message* copyMsg(struct message* src);
void freeMessage(struct message* msg);
#endif