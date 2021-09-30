#ifndef CLIENT_H
#define CLIENT_H
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <curses.h>
#include <pthread.h>
#include <dirent.h>
#include <locale.h>
#include "common.h"

enum screenView {
    FOLDER,
    FILEREAD,
    INPUT
};

void clientMode(char* ip_addr, int port);
#endif