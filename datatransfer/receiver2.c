#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <time.h>
#include "packet.h"

//#define PORT 5005
#define BUF_SIZE 200
#define FILE_NAME "received_file"
#define RESEND_TIME 2
//#define LOSS_PROBABILITY 0.2 // 패킷 손실 확률 0.2 == 20%

int sockfd;
Packet packet;
struct sockaddr_in servaddr, cliaddr;
socklen_t len;
FILE *file;
FILE *log_file;
Packet ackPacket;
char filename[500];


int main(int argc, char *argv[]) {
    if (argc != 3){
        printf("USE: %s <port> <drop probability:0.x = x percent>\n", argv[0]);
        exit(1);
    }

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
    servaddr.sin_port = htons(atoi(argv[1]));

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

    log_file = fopen("receiver_log.txt", "w");

    int already_received = -1;

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
            print_event("[reveiver - Packet Received]", &packet, log_file);
            break;
        }

        // 데이터 패킷 로스
        if((double)rand() / RAND_MAX < atof(argv[2])){
            print_event("[receiver - Data Packet Loss]", &packet, log_file);
        }else{
            // 이미 받은 패킷인지 확인
            if (packet.seqNum <= already_received){
                ackPacket.type = ACK;
                ackPacket.seqNum = packet.seqNum + 1;
                ackPacket.ackNum = already_received;
                ackPacket.length = 0;
                sendto(sockfd, &ackPacket, sizeof(Packet), 0, (struct sockaddr *)&cliaddr, len);
                print_event("[receiver - Already received Packet; send ACK]", &ackPacket, log_file);
            }else{
                // 받은 패킷 출력
                print_event("[receiver - Packet Received]", &packet, log_file);
                fwrite(packet.data, sizeof(char), packet.length, file);

                already_received = packet.seqNum;

                ackPacket.type = ACK;
                ackPacket.ackNum = packet.seqNum;
                ackPacket.seqNum = ++packet.seqNum;
                ackPacket.length = 0;
                sendto(sockfd, &ackPacket, sizeof(Packet), 0, (struct sockaddr *)&cliaddr, len);
                print_event("[receiver - ACK Sent]", &ackPacket, log_file);
                
            }

        }
    }

    printf("파일 수신 완료\n");

    fclose(file);
    close(sockfd);
    return 0;
}
