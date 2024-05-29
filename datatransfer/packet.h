// packet.h
#ifndef PACKET_H
#define PACKET_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <stdio.h>

#define DATA 0
#define ACK 1
#define EOT 2

typedef struct Packet {
    int type;
    int seqNum;
    int ackNum;
    int length;
    char data[1000];
} Packet;

void print_event(const char *event, Packet *pkt, FILE *log_file) {
    time_t now = time(NULL);
    printf("%sEvent: %s, Type: %d, SeqNum: %d, AckNum: %d, Length: %d\n", ctime(&now), event, pkt->type, pkt->seqNum, pkt->ackNum, pkt->length);
    fprintf(log_file, "%sEvent: %s, Type: %d, SeqNum: %d, AckNum: %d, Length: %d\n", ctime(&now), event, pkt->type, pkt->seqNum, pkt->ackNum, pkt->length);
}

#endif // PACKET_H
