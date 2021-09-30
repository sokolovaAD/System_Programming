#include "opts.h"
#include "client.h"
#include "server.h"


int main(int argc, char** argv) {
    struct params options;
    getOptions(argc, argv, &options);
    if (options.mode == 1) clientMode(options.clientOpt.ip, options.port);
    else
     if (options.mode == 2) serverMode(options.serverOpt.root, options.port);
    return 0;
}