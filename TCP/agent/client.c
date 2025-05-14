#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

struct sockaddr_in serv_addr;

int main(int argc, char *argv[])
{
    if (argc < 3) {
        fprintf(stderr, "Usage: %s <server_ip> <server_port> <message>\n", argv[0]);
        exit(1);
    } else if(argc == 3) {
        argv[3] = "Hello server !";
    }

    int sockfd;
    if ((sockfd = socket(PF_INET, SOCK_STREAM, 0)) < 0) {
        perror("socket");
        exit(1);
    }

    bzero( (char *) &serv_addr, sizeof(serv_addr) );
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_port = htons((unsigned short)atoi(argv[2]));
    serv_addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sockfd, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0) {
        perror("cliecho : erreur connect");
        close(sockfd);
        exit(1);
    }

    // Send a message to the server
    char *message = argv[3];
    if (send(sockfd, message, strlen(message), 0) < 0) {
        perror("ERROR writing to socket");
        close(sockfd);
        exit(1);
    }

    // Read the response from the server
    char buffer[256];
    memset(buffer, 0, 256);
    if (read(sockfd, buffer, 255) < 0) {
        perror("ERROR reading from socket");
        close(sockfd);
        exit(1);
    }
    printf("Server response: %s\n", buffer);
    

    close(sockfd);
    return 0;
}