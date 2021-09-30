#include "opts.h"

void getOptions(int argc, char** argv, struct params* opts) {
    int c;
    opts->mode = 0;
    while ((c = getopt(argc, argv, "csd:a:p:")) != -1) {
        switch (c) {
            case 'c': opts->mode = 1; break;
            case 's': opts->mode = 2; break;
            case 'd': strcpy(opts->serverOpt.root, optarg); break;
            case 'a': strcpy(opts->clientOpt.ip, optarg); break;
            case 'p': opts->port = strtol(optarg, NULL, 10); break;

        }
    }
}

