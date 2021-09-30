#ifndef OPTS_H
#define OPTS_H
#include <stdio.h>
#include <stdlib.h>
#include <limits.h>
#include <inttypes.h>
#include <getopt.h>
#include <string.h>


struct serverOpt {
    char root[PATH_MAX];
};

struct clientOpt {
    char ip[16];
};

struct params {
    int mode;
    int port;
    struct serverOpt serverOpt;
    struct clientOpt clientOpt;
};

void getOptions(int argc, char** argv, struct params* opts);
#endif