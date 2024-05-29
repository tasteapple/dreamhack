#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "packet.h"

#define PORT 5005
#define BUF_SIZE 30
#define FILE_NAME "received_file.txt"
#define RESEND_TIME 2
#define LOSS_PROBABILITY 0.2 // 패킷 손실 확률 0.2 == 20%

int main() {
    int sockfd;
    Packet packet;
    struct sockaddr_in servaddr, cliaddr;
    socklen_t len;
    FILE *file;

    srand(time(NULL)); // 난수 초기화

    // 소켓 생성
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&servaddr, 0, sizeof(servaddr));
    memset(&cliaddr, 0, sizeof(cliaddr));

    // 서버 주소 정보 설정
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = INADDR_ANY;
    servaddr.sin_port = htons(PORT);

    // 소켓 바인딩
    if (bind(sockfd, (const struct sockaddr *)&servaddr, sizeof(servaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("서버가 실행 중입니다...\n");

    // 파일 열기
    file = fopen(FILE_NAME, "wb");
    if (file == NULL) {
        perror("파일 열기 실패");
        close(sockfd);
        exit(EXIT_FAILURE);
    }


    // 데이터 수신 및 ACK 전송
    while (1) {
        len = sizeof(cliaddr);
        int n = recvfrom(sockfd, &packet, sizeof(Packet), 0, (struct sockaddr *)&cliaddr, &len);
        if (n < 0) {
            perror("recvfrom error");
            break;
        }

        // 전송 종료 신호 확인
        if (packet.type == EOT) {
            print_event("Packet Received", &packet);
            //printf("파일 전송 완료\n");
            break;
        }
        if((double)rand() / RAND_MAX < LOSS_PROBABILITY){
            print_event("Packet Loss", &packet);
        }else{
            // 받은 패킷 번호 출력
            print_event("Packet Received", &packet);

            fwrite(packet.data, sizeof(char), packet.length, file);

            // ACK 패킷 준비 및 전송
            Packet ackPacket;
            ackPacket.type = ACK;
            ackPacket.ackNum = packet.seqNum;
            ackPacket.seqNum = ++packet.seqNum;
            ackPacket.length = 0;
            sendto(sockfd, &ackPacket, sizeof(Packet), 0, (struct sockaddr *)&cliaddr, len);
            print_event("ACK Sent", &ackPacket);
            //printf("ACK 전송 완료\n");
        }
    }

    fclose(file);
    close(sockfd);
    return 0;
}
