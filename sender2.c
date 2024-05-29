#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <sys/time.h>
#include <time.h>
#include <signal.h>
#include "packet.h"

#define PORT 5005
#define BUF_SIZE 30
#define TIMEOUT 2 // 소켓 연결 타임아웃 시간
#define FILE_NAME "test_file.txt"
#define RESEND_TIME 2
#define LOSS_PROBABILITY 0.2 // 패킷 손실 확률 0.2 == 20%

int sockfd;
Packet packet;
struct sockaddr_in servaddr;
struct timeval tv;
socklen_t len;
FILE *file;
int seqNum = 0;

void send_packet() {
    sendto(sockfd, &packet, sizeof(Packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    print_event("Send", &packet);
    alarm(RESEND_TIME);
}

void sigalrm_handler(int sig) {
    print_event("Timeout Resend", &packet);
    send_packet();
    alarm(RESEND_TIME);
}

int main() {
    /*
    //int sockfd;
    //Packet packet;
    //struct sockaddr_in servaddr;
    //struct timeval tv;
    //socklen_t len;
    //FILE *file;
    //int seqNum = 0;
    */

    Packet ackPacket;

    srand(time(NULL)); // 난수 초기화

    signal(SIGALRM, sigalrm_handler);

    // 소켓 생성
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));

    // 서버 주소 정보 설정
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(PORT);
    servaddr.sin_addr.s_addr = INADDR_ANY;

    // 소켓 연결 타임아웃 설정
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error setting socket options");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 파일 열기
    file = fopen(FILE_NAME, "rb");
    if (file == NULL) {
        perror("파일 열기 실패");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("데이터 전송 중...\n");

    // 데이터 전송 및 ACK 수신
    while ((packet.length = fread(packet.data, sizeof(char), BUF_SIZE, file)) > 0) {
        packet.type = DATA;  // 데이터 패킷
        packet.seqNum = seqNum;
        packet.ackNum = 0;

        while (1) {
            /*
            printf("데이터 전송 중...\n");
            sendto(sockfd, &packet, sizeof(Packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
            print_event("Send", &packet);
            
            alarm(RESEND_TIME); // 패킷 보내고 타임아웃 설정
            */

            send_packet();

            len = sizeof(servaddr);
            int n = recvfrom(sockfd, &ackPacket, sizeof(Packet), 0, (struct sockaddr *)&servaddr, &len);

            if (n < 0) {
                sleep(RESEND_TIME);
            } else {
                //if ((double)rand() / RAND_MAX < LOSS_PROBABILITY);
                if (ackPacket.type == ACK && ackPacket.ackNum == seqNum) {
                    print_event("ACK Received", &ackPacket);
                    alarm(0); // ACK 받으면 타이머 해제
                    break;  // ACK를 받으면 다음 데이터 전송
                }
            }
        }

        seqNum++;
    }

    // 전송 종료 신호 전송
    packet.type = EOT;  // 종료 패킷
    packet.seqNum = seqNum;
    packet.length = 0;
    sendto(sockfd, &packet, sizeof(Packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    print_event("Send", &packet);
    printf("파일 전송 완료\n");

    fclose(file);
    close(sockfd);
    return 0;
}
