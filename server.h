#ifndef SERV_H
#define SERV_H
#include <stdio.h>
#include <stdlib.h>
#include <inttypes.h>
#include <string.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <limits.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>
#include <dirent.h>
#include "common.h"

void serverMode(char* rootPath, int port);
#endif