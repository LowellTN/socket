#include <sys/types.h>
#include <sys/socket.h>
#include <sys/timeb.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>

#define BUFFER_SIZE 256

int main(int argc, char *argv[])
{
    if (argc < 3)
    {
        fprintf(stderr, "Usage: %s <server_ip> <server_port>\n", argv[0]);
        exit(1);
    }

    int sockfd;
    struct sockaddr_in serv_addr;
    char buffer[BUFFER_SIZE];
    socklen_t serv_len;

    if ((sockfd = socket(AF_INET, SOCK_DGRAM, 0)) < 0)
    {
        perror("socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons(atoi(argv[2]));
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

    struct timeb tb;
    if (ftime(&tb) != 0)
    {
        perror("ftime");
        close(sockfd);
        exit(1);
    }

    struct tm *local_time = localtime(&tb.time);
    if (local_time == NULL)
    {
        perror("localtime");
        close(sockfd);
        exit(1);
    }

    snprintf(buffer, BUFFER_SIZE, "Current hour: %02d:%02d:%02d.%03d", local_time->tm_hour, local_time->tm_min, local_time->tm_sec, tb.millitm);

    serv_len = sizeof(serv_addr);
    if (sendto(sockfd, buffer, strlen(buffer), 0, (struct sockaddr *)&serv_addr, serv_len) < 0) {
        perror("sendto");
        close(sockfd);
        exit(1);
    }

    printf("Sent to server: %s\n", buffer);

    close(sockfd);
    return 0;
}