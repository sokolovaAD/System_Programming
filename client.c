//#ifndef CL_H
//#define CL_H
#include "client.h"

 char currentPath[PATH_MAX] = {'\0'};
 char downloaded_filename[PATH_MAX] = {'\0'};
 char notification[MAX_DATA_SIZE] = {'\0'};
struct message* currentFolder;
int selectedEntry;

 void menu() {
    printw("d - Download.\n");
    printw("u - Upload.\n");
    printw("e - Go to path or view file.\n");
    printw("q - Exit.\n");
    if (notification[0] != '\0') {
        printw("\nNotification: %s\n\n", notification);
    } else {
        printw("\n\n\n");
    }
    refresh();
}

 size_t elementSize(struct dirent* dirent) {
    return sizeof(dirent->d_type) +
           sizeof(dirent->d_ino) +
           sizeof(dirent->d_reclen) +
           sizeof(dirent->d_off) +
           sizeof(dirent->d_reclen) +
           dirent->d_reclen + 1;
}

 void sendHello(int sd) {
    struct message* msg = malloc(sizeof(struct message));
    struct message* resp_msg;
     msg->command = HELLO;

    sendMsg(msg, sd);
    resp_msg = receiveMsg(sd);

    if (strncmp(resp_msg->data, HANDSHAKE, strlen(HANDSHAKE)) != 0) {
        printf("Error in handshake\n");
        return;
    }

    strcpy(currentPath, resp_msg->path);
    free(msg);
    freeMessage(resp_msg);
}

 void ls(int sd, char* path) {
    struct message* msg = malloc(sizeof(struct message));

     msg->command = LS;
    strcpy(msg->path, path);
     msg->size = 0;
     msg->data = NULL;
     sendMsg(msg, sd);
    free(msg);
}

 void cd(int sd, char* path) {
    struct message* cd = calloc(1, sizeof(struct message));

     cd->command = CD;
     cd->size = strlen(currentPath) + strlen(path) + 2;
     cd->data = calloc(1, cd->size);

    strcpy(cd->path, currentPath);
    strcpy(cd->data, currentPath);
    strcat(cd->data, "/");
    strcat(cd->data, path);

     sendMsg(cd, sd);
     freeMessage(cd);
}

 void readFile(int sd, char* path) {
    struct message* msg = calloc(1, sizeof(struct message));

     msg->command = CAT;
     msg->size = strlen(currentPath) + strlen(path) + 2;
     msg->data = calloc(1, msg->size);

    strcpy(msg->path, currentPath);
    strcpy(msg->data, currentPath);
    strcat(msg->data, "/");
    strcat(msg->data, path);

     sendMsg(msg, sd);
     freeMessage(msg);
}

 void download(int sd, char* filename) {
    struct message* msg = calloc(1, sizeof(struct message));

     msg->command = DOWNLOAD;
     msg->size = strlen(currentPath) + strlen(filename) + 2;
     msg->data = calloc(1, msg->size);

    strcpy(msg->path, currentPath);
    strcpy(msg->data, currentPath);
    strcat(msg->data, "/");
    strcat(msg->data, filename);

     sendMsg(msg, sd);
     freeMessage(msg);
}

 void upload(int sd, char* filename) {
    struct message* msg = calloc(1, sizeof(struct message));
    char buffer[MAX_DATA_SIZE] = {'\0'};
    char* ptr = filename + strlen(filename);
    size_t bufSize;

    FILE* file = fopen(filename, "rb");

    msg->data = NULL;

    if (file != NULL) {
        msg->command = UPLOAD;
        strcpy(msg->path, currentPath);
        strcat(msg->path, "/");
        while (*--ptr != '/') {}
        *ptr = '\0';
        strcat(msg->path, ++ptr);

        msg->size = 0;
        msg->data = malloc(1);

        while (fread(buffer, MAX_DATA_SIZE, 1, file) > 0) {
            msg->data = realloc(msg->data, msg->size + MAX_DATA_SIZE);
            strncpy(msg->data + msg->size, buffer, MAX_DATA_SIZE);
            msg->size += MAX_DATA_SIZE;
        }
        bufSize = strlen(buffer);
        msg->data = realloc(msg->data, msg->size + bufSize);
        strncpy(msg->data + msg->size, buffer, bufSize);
        msg->size += bufSize;

        sendMsg(msg, sd);
        fclose(file);
    } else {
        strcpy(notification, "Can't open file ");
        strcat(notification, filename);
    }

     freeMessage(msg);
}

 struct dirent* getElement() {
    size_t data_ptr = 0, i = 0;
    struct dirent* element = malloc(sizeof(struct dirent));

    while(data_ptr < currentFolder->size) {
        memcpy(element, currentFolder->data + data_ptr, sizeof(struct dirent));
        if (i++ == selectedEntry) {
            return element;
        }

        data_ptr += elementSize(element);
    }
    return NULL;
}

 char* getSelectedName() {
    struct dirent* entry = getElement();
    char* name = calloc(1, entry->d_reclen + (entry->d_type == DT_DIR ? 2 : 1));
    strcpy(name, entry->d_name);
    if (entry->d_type == DT_DIR) {
        strcat(name, "/");
    }
    free(entry);
    return name;
}

 unsigned char getType() {
    struct dirent* element = getElement();
    unsigned char type = element->d_type;
    free(element);
    return type;
}

 void printFolder(int selected) {
    size_t ptr = 0, i = 0;
    struct dirent* element = malloc(sizeof(struct dirent));
    bool printed = false;

     selectedEntry += selected;
    clear();
    menu();
    if (selectedEntry < 0) selectedEntry = 0;
    while(ptr < currentFolder->size) {
        memcpy(element, currentFolder->data + ptr, sizeof(struct dirent));
        if (i++ == selectedEntry) {
            printw("> %s", element->d_name);
            printed = true;
        } else printw("  %s", element->d_name);

        if (element->d_type == DT_DIR) printw("/\n");
        else printw("\n");

        ptr += elementSize(element);
    }
    free(element);
    if (printed == false) printFolder(-1);
    refresh();
}

 void printFileContent(struct message* file_message) {
    file_message->data[file_message->size] = '\0';
    clear();
    printw("To go back press 'r'\n\n");
    printw("%s", file_message->data);
    refresh();
}

 void saveDownloadedFile(struct message* message) {
    FILE* file;
    char filename[PATH_MAX] = {'\0'};
    char buffer[MAX_DATA_SIZE] = {'\0'};
    unsigned long long data_ptr = 0;

    strcpy(filename, getenv("HOME"));
    strcat(filename, "/Downloads/");
    strcat(filename, downloaded_filename);
    file = fopen(filename, "wb");

    if (file != NULL) {

        while (data_ptr < message->size) {
            unsigned long long int min = MAX_DATA_SIZE > message->size ? message->size : MAX_DATA_SIZE;
            strncpy(buffer, message->data + data_ptr, min);
            fwrite(buffer, min, 1, file);
            data_ptr += MAX_DATA_SIZE;
        }
        strcat(notification, downloaded_filename);
        strcat(notification, " was saved as ");
        strcat(notification, filename);

        fclose(file);
    } else {
        endwin();
        puts(filename);
        puts("Can't download file");
        exit(EXIT_FAILURE);
    }
}

 void serve(int sd) {
    int c, i;
    char* name;
    char inputFilename[PATH_MAX] = {'\0'};
    enum screenView screenView = FOLDER;

    do {
        c = getch();

        if (screenView == INPUT) {
            i = 0;
            while (c != '\n') {
                inputFilename[i++] = c;
                c = getch();
            }
            inputFilename[PATH_MAX - 1] = '\0';
            inputFilename[i] = '\0';
            upload(sd, inputFilename);
            screenView = FOLDER;
            noecho();
            cbreak();
            printFolder(0);
            continue;
        }

        switch (c) {
            case 'r':
                if (screenView == FILEREAD) {
                    ls(sd, currentPath);
                    screenView = FOLDER;
                }
                break;
            case 'w':
                if (screenView == FOLDER)
                    printFolder(-1);
                break;
            case 's':
                if (screenView == FOLDER)
                    printFolder(1);

                break;
            case 'd':
                if (screenView == FOLDER) {
                    name = getSelectedName();
                    if (getType() == DT_REG) {
                        strcpy(downloaded_filename, name);
                        download(sd, name);
                    }
                    free(name);
                }
                break;
            case 'u':
                if (screenView == FOLDER) {
                    screenView = INPUT;
                    echo();
                    nocbreak();
                }
                break;
            case 'e':
                if (screenView == FOLDER) {
                    name = getSelectedName();
                    if (getType() == DT_DIR)
                        cd(sd, name);
                    else {
                        readFile(sd, name);
                        screenView = FILEREAD;
                    }
                    free(name);
                }
                break;
        }
    } while (c != 'q');
}

void* receiveHandler(void* sd) {
    int socket = *(int*) sd;
    struct message* recv_message = NULL;
    do {
        freeMessage(recv_message);
        recv_message = receiveMsg(socket);
        switch (recv_message->command) {
            case CD:
                strcpy(currentPath, recv_message->path);
                ls(socket, currentPath);
                break;
            case LS:
                currentFolder = copyMsg(recv_message);
                selectedEntry = 0;
                printFolder(0);
                break;
            case CAT:
                printFileContent(recv_message);
                break;
            case DOWNLOAD:
                saveDownloadedFile(recv_message);
                printFolder(0);
                break;
            case UPDATE:
                ls(socket, currentPath);
                break;
            case EXIT:
                endwin();
                puts("Server closed");
                close(socket);
                exit(0);
            case ERROR:
            case HELLO:
            case UPLOAD:
                break;
        }
    } while(recv_message->command != EXIT);

    return 0;
}

void clientMode(char* ip_addr, int port) {
    int socket_desc;
    int* p_socket = malloc(1);
    struct sockaddr_in server;
    pthread_t receive_thread;
    server.sin_addr.s_addr = inet_addr(ip_addr);
    server.sin_family = AF_INET;
    server.sin_port = htons(port);

    if ((socket_desc = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
        perror("Socket creation error");
        return;
    }
    if (connect(socket_desc, (struct sockaddr*) &server, sizeof(server)) < 0) {
        perror("Connection error");
        return;
    }

    sendHello(socket_desc);
    *p_socket = socket_desc;
    if (pthread_create(&receive_thread, NULL, receiveHandler, (void *) p_socket) != 0) {
        perror("Couldn't create receiver thread");
        return;
    }

    initscr();
    noecho();
    menu();
    ls(socket_desc, currentPath);
    serve(socket_desc);
    endwin();
}
//#endif