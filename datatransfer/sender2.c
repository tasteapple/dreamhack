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
#define BUF_SIZE 200
#define TIMEOUT 2 // 소켓 연결 타임아웃 시간
//#define FILE_NAME "test_file.txt"
#define RESEND_TIME 2
//#define LOSS_PROBABILITY 0.2 // 패킷 손실 확률 0.2 == 20%

int sockfd;
Packet packet;
struct sockaddr_in servaddr, myaddr;
struct timeval tv;
socklen_t len;
FILE *file;
FILE *log_file;
int seqNum = 0;
int resend = 0;
Packet ackPacket;
int resend_time;

void send_packet(int num) {
    sendto(sockfd, &packet, sizeof(Packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    // 재전송인지 처음 전송인지 ACK Loss인지에 따른 이벤트 출력
    if (num == 0)
        print_event("[sender - Send]", &packet, log_file);
    else if (num == 1)
        print_event("[sender - ACK Loss; resend previous Packet]", &ackPacket, log_file);
    
    alarm(RESEND_TIME);
}

void sigalrm_handler(int sig) {
    print_event("[sender - Timeout Packet; resend Packet]", &packet, log_file);
}

int main(int argc, char *argv[]) {
    if (argc != 7){
        printf("USE: %s <sender port> <receiver ip> <receiver port> <timeout interval:x = x sec> <filename> <drop probability:0.x = x percent>\n", argv[0]);
        exit(1);
    }

    int sender_port = atoi(argv[1]);
    char *receiver_ip = argv[2];
    int receiver_port = atoi(argv[3]);
    resend_time = atoi(argv[4]);
    char *filename = argv[5];
    double drop_probability = atof(argv[6]);

    signal(SIGALRM, sigalrm_handler); // 알람 시그널 설정
    srand(time(NULL)); // 난수 초기화
    log_file = fopen("log.txt", "a");

    // 소켓 생성
    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {
        perror("socket creation failed");
        exit(EXIT_FAILURE);
    }

    memset(&myaddr, 0, sizeof(myaddr));
    myaddr.sin_family = AF_INET;
    myaddr.sin_port = htons(sender_port);
    myaddr.sin_addr.s_addr = INADDR_ANY;

    if (bind(sockfd, (const struct sockaddr *)&myaddr, sizeof(myaddr)) < 0) {
        perror("bind failed");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    // 서버 주소 정보 설정
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(receiver_port);
    if (inet_pton(AF_INET, receiver_ip, &servaddr.sin_addr) <= 0) {
        perror("Invalid address/ Address not supported");
        exit(EXIT_FAILURE);
    }

    // 소켓 연결 타임아웃 설정
    tv.tv_sec = TIMEOUT;
    tv.tv_usec = 0;
    if (setsockopt(sockfd, SOL_SOCKET, SO_RCVTIMEO, &tv, sizeof(tv)) < 0) {
        perror("Error setting socket options");
        close(sockfd);
        exit(EXIT_FAILURE);
    }


    // 파일 열기
    file = fopen(filename, "rb");
    if (file == NULL) {
        perror("파일 열기 실패");
        close(sockfd);
        exit(EXIT_FAILURE);
    }

    printf("데이터 전송 중...\n");

    //int acknum = -1;

    // 데이터 전송 및 ACK 수신
    while ((packet.length = fread(packet.data, sizeof(char), BUF_SIZE, file)) > 0) {
        packet.type = DATA;  // 데이터 패킷
        packet.seqNum = seqNum;
        packet.ackNum = 0;

        while (1) {
            send_packet(resend);

            len = sizeof(servaddr);
            int n = recvfrom(sockfd, &ackPacket, sizeof(Packet), 0, (struct sockaddr *)&servaddr, &len);

            // 데이터 패킷 로스
            if (n < 0) {
                sleep(RESEND_TIME+1);
                resend = 0;
            } else {
                // ACK 패킷 로스
                if ((double)rand() / RAND_MAX < drop_probability){
                    alarm(0); // ACK 로스시 타이머 해제
                    resend = 1;
                }else{
                    // ACK 패킷 받음
                    if (ackPacket.type == ACK && ackPacket.ackNum == seqNum) {
                        print_event("[sender - ACK Received]", &ackPacket, log_file);
                        alarm(0); // ACK 받으면 타이머 해제
                        resend = 0;
                        break;  // ACK를 받으면 다음 데이터 전송
                    }
                }
            }
            alarm(0);
        }

        seqNum++;
        resend = 0;
    }

    // 전송 종료 신호 전송
    packet.type = EOT;  // 종료 패킷
    packet.seqNum = seqNum;
    packet.length = 0;
    sendto(sockfd, &packet, sizeof(Packet), 0, (const struct sockaddr *)&servaddr, sizeof(servaddr));
    print_event("[sender - Send]", &packet, log_file);
    printf("파일 전송 완료\n");

    fclose(file);
    close(sockfd);
    return 0;
}
