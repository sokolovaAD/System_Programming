#include "common.h"

void sendMsg(struct message* msg, int sd) {
    if (write(sd, msg, sizeof(struct message)) < 0) return;
    if (write(sd, msg->data, msg->size * sizeof(char)) < 0) return;
}

struct message* receiveMsg(int sd) {
    struct message* msg = calloc(1, sizeof(struct message));
    read(sd, msg, sizeof(struct message));
    msg->data = malloc(msg->size * sizeof(char));
    read(sd, msg->data, msg->size);
    return msg;
}

struct message* copyMsg(struct message* src) {
    struct message* dst = NULL;
    dst = calloc(1, sizeof(struct message));
    dst->data = calloc(1, src->size);
    memcpy(dst, src, sizeof(struct message) + src->size);
    return dst;
}

void freeMessage(struct message* msg) {
    if (msg == NULL) return;
    if (msg->data != NULL)
        free(msg->data);
    free(msg);
}