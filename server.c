#include "server.h"

 char root[PATH_MAX] = {'\0'};

 inline size_t elementSize(struct dirent* dirent) {
    return sizeof(dirent->d_type) +
           sizeof(dirent->d_ino) +
           sizeof(dirent->d_reclen) +
           sizeof(dirent->d_off) +
           sizeof(dirent->d_reclen) +
           dirent->d_reclen + 1;
}

 bool belongsRoot(char* dir) {
    if (strlen(dir) < strlen(root)) return false;
    return strncmp(dir, root, strlen(root)) == 0;
}

 struct message* doHello(struct message* message) {
    struct message* outputMsg = malloc(sizeof(struct message));
     outputMsg->command = HELLO;
     outputMsg->data = malloc(strlen(HANDSHAKE) + 1);
    strcpy(outputMsg->data, HANDSHAKE);
     outputMsg->size = strlen(HANDSHAKE) + 1;
    strcpy(outputMsg->path, root);
    return outputMsg;
}

 struct message* doCd(struct message* message) {
    struct message* outputMsg = calloc(1, sizeof(struct message));
     outputMsg->size = 0;
    char real_path[PATH_MAX] = {'\0'};
    char selected_dir[PATH_MAX] = {'\0'};
    strncpy(selected_dir, message->data, message->size);
    if (selected_dir[0] == '/') {
        realpath(selected_dir, real_path);
    } else {
        strcat(selected_dir, message->path);
        strncpy(selected_dir, message->data, message->size);
        realpath(selected_dir, real_path);
    }
    if (belongsRoot(real_path)) {
        outputMsg->command = CD;
        strcpy(outputMsg->path, real_path);
    } else {
        outputMsg->command = ERROR;
    }
    return outputMsg;
}

 struct message* doLs(struct message* message) {
    struct message* outputMsg = malloc(sizeof(struct message));
    DIR* dir;
    struct dirent* entry;
    size_t entrySize;
    size_t data_ptr = 0;
    outputMsg->size = 0;
    outputMsg->data = malloc(1);
    strcpy(outputMsg->path, message->path);

    if (belongsRoot(message->path)) {
        outputMsg->command = LS;
        dir = opendir(message->path);
        if (!dir) {
            outputMsg->command = ERROR;
            return outputMsg;
        }
        while ((entry = readdir(dir)) != NULL) {
            entrySize = elementSize(entry);
            data_ptr = outputMsg->size;
            outputMsg->size += entrySize;
            outputMsg->data = realloc(outputMsg->data, outputMsg->size);
            memcpy(outputMsg->data + data_ptr, entry, entrySize);
        }
    } else {
        outputMsg->command = ERROR;
    }
    return outputMsg;
}

 struct message* doCat(struct message* message) {
    struct message* outputMsg = malloc(sizeof(struct message));
    char filename[PATH_MAX] = {'\0'};
    char buf[MAX_DATA_SIZE] = {'\0'};
    size_t bufSize;
    FILE* file;

     outputMsg->size = 0;
     outputMsg->data = malloc(1);

    if (message->data[0] == '/')
        strncpy(filename, message->data, message->size);
    else {
        strcat(filename, message->path);
        strncat(filename, message->data, message->size);
    }
    if (belongsRoot(filename)) {
        outputMsg->command = CAT;
        file = fopen(filename, "rb");
        while (fread(buf, MAX_DATA_SIZE, 1, file) > 0) {
            outputMsg->data = realloc(outputMsg->data, outputMsg->size + MAX_DATA_SIZE);
            strncpy(outputMsg->data + outputMsg->size, buf, MAX_DATA_SIZE);
            outputMsg->size += MAX_DATA_SIZE;
        }
        bufSize = strlen(buf);
        outputMsg->data = realloc(outputMsg->data, outputMsg->size + bufSize);
        strncpy(outputMsg->data + outputMsg->size, buf, bufSize);
        outputMsg->size += bufSize;
        fclose(file);
    } else
        outputMsg->command = ERROR;
    return outputMsg;
}

 struct message* doUpload(struct message* message) {
    struct message* outputMsg = malloc(sizeof(struct message));
    char buf[MAX_DATA_SIZE] = {'\0'};
    unsigned long long ptr = 0;
    FILE* file = fopen(message->path, "wb");

    if (belongsRoot(message->path) && file != NULL) {

        while(ptr < message->size) {
            unsigned long long int min = MAX_DATA_SIZE > message->size ? message->size : MAX_DATA_SIZE;
            strncpy(buf, message->data + ptr, min);
            fwrite(buf, min, 1, file);
            ptr += MAX_DATA_SIZE;
        }
        outputMsg->command = UPDATE;
        outputMsg->data = NULL;
        fclose(file);
    } else
        outputMsg->command = ERROR;
    return outputMsg;
}

 struct message* doDownload(struct message* message) {
    struct message* outputMsg = malloc(sizeof(struct message));
    char filename[PATH_MAX] = {'\0'};
    char buf[MAX_DATA_SIZE] = {'\0'};
    size_t bufSize;
    FILE* file;

    outputMsg->size = 0;
    outputMsg->data = malloc(1);

    if (message->data[0] == '/')
        strncpy(filename, message->data, message->size);
    else {
        strcat(filename, message->path);
        strncat(filename, message->data, message->size);
    }

    if (belongsRoot(filename)) {
        outputMsg->command = DOWNLOAD;
        file = fopen(filename, "rb");
        while (fread(buf, MAX_DATA_SIZE, 1, file) > 0) {
            outputMsg->data = realloc(outputMsg->data, outputMsg->size + MAX_DATA_SIZE);
            strncpy(outputMsg->data + outputMsg->size, buf, MAX_DATA_SIZE);
            outputMsg->size += MAX_DATA_SIZE;
        }
        bufSize = strlen(buf);
        outputMsg->data = realloc(outputMsg->data, outputMsg->size + bufSize);
        strncpy(outputMsg->data + outputMsg->size, buf, bufSize);
        outputMsg->size += bufSize;
        fclose(file);
    } else
        outputMsg->command = ERROR;
    return outputMsg;
}

 struct message* handleMessage(struct message* message) {
    switch (message->command) {
        case CD:
            return doCd(message);
        case LS:
            return doLs(message);
        case CAT:
            return doCat(message);
        case UPLOAD:
            return doUpload(message);
        case DOWNLOAD:
            return doDownload(message);
        case HELLO:
            return doHello(message);
    }
    return NULL;
}

void* charReadThread(void* sd) {
    char input;
    do {
        scanf("%c", &input);
    } while (input != 'q');
    close(*(int*) sd);
    exit(0);
}

void serverMode(char* rootPath, int port) {
    int mainSd, c, new_socket;
    int* server_socket = malloc(1);
    struct sockaddr_in server, client;
    struct message *inputMsg, *outputMsg;
    fd_set openSet, read_fd_set;
    pthread_t exit_thread;

    server.sin_family = AF_INET;
    server.sin_addr.s_addr = INADDR_ANY;
    server.sin_port = htons(port);

    if ((mainSd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation failed");
        return;
    }

    if ((bind(mainSd, (struct sockaddr*) &server, sizeof(server))) < 0) {
        perror("Failed to bind");
        return;
    }

    listen(mainSd, 128);
    strcpy(root, rootPath);
    *server_socket = mainSd;

    pthread_create(&exit_thread, NULL, charReadThread, (void *) server_socket);

    FD_ZERO(&openSet);
    FD_SET(mainSd, &openSet);

    c = sizeof(struct sockaddr_in);

    while (true) {
        read_fd_set = openSet;

        if (select(FD_SETSIZE, &read_fd_set, NULL, NULL, NULL) < 0) {
            perror("error with select");
            return;
        }

        for (int i = 0; i < FD_SETSIZE; ++i) {
            if (FD_ISSET(i, &read_fd_set)) {
                if (i == mainSd) {
                    new_socket = accept(mainSd, (struct sockaddr*) &client, (socklen_t * ) & c);
                    if (new_socket < 0) {
                        perror("accept failed");
                        return;
                    }
                    FD_SET(new_socket, &openSet);
                    inputMsg = receiveMsg(new_socket);
                    outputMsg = handleMessage(inputMsg);
                    sendMsg(outputMsg, new_socket);
                    freeMessage(inputMsg);
                    freeMessage(outputMsg);
                } else {
                    inputMsg = receiveMsg(i);
                    if (inputMsg->command != EOSIG) {
                        outputMsg = handleMessage(inputMsg);
                        if (inputMsg->command == UPLOAD && outputMsg->command == UPDATE)
                            for (int j = 4; j < FD_SETSIZE; ++j)
                                sendMsg(outputMsg, j);
                        else sendMsg(outputMsg, i);
                        freeMessage(outputMsg);
                    } else {
                        close(i);
                        FD_CLR(i, &openSet);
                    }
                    freeMessage(inputMsg);
                }
            }
        }
    }
}