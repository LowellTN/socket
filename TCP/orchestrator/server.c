#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>

#define SERV_PORT 2222

int dialogSocket;
int serverSocket;
socklen_t clilen;

struct sockaddr_in cli_addr;
struct sockaddr_in serv_addr;

int main(int argc, char *argv[])
{
    if ((serverSocket = socket(PF_INET, SOCK_STREAM, 0)) < 0)
    {
        perror("socket");
        exit(1);
    }

    memset(&serv_addr, 0, sizeof(serv_addr));
    serv_addr.sin_family = AF_INET;
    serv_addr.sin_addr.s_addr = htonl(INADDR_ANY);
    serv_addr.sin_port = htons(SERV_PORT);

    if (bind(serverSocket, (struct sockaddr *)&serv_addr, sizeof(serv_addr)) < 0)
    {
        perror("servecho: erreur bind\n");
        close(serverSocket);
        exit(1);
    }

    if (listen(serverSocket, SOMAXCONN) < 0)
    {
        perror("servecho: erreur listen\n");
        close(serverSocket);
        exit(1);
    }

    printf("Server is listening on port %d\n", SERV_PORT);

    clilen = sizeof(cli_addr);
    dialogSocket = accept(serverSocket, (struct sockaddr *)&cli_addr, &clilen);
    if (dialogSocket < 0)
    {
        perror("servecho : erreur accept\n");
        close(serverSocket);
        exit(1);
    }

    // Handle the client connection here
    char buffer[256];
    int n;
    memset(buffer, 0, 256);
    n = read(dialogSocket, buffer, 255);
    if (n < 0) 
    {
        perror("ERROR reading from socket");
        close(dialogSocket);
        close(serverSocket);
        exit(1);
    }
    printf("Here is the message: %s\n", buffer);

    n = send(dialogSocket, "I got your message", 18, 0);
    if (n < 0) 
    {
        perror("ERROR writing to socket");
        close(dialogSocket);
        close(serverSocket);
        exit(1);
    }

    close(dialogSocket);
    close(serverSocket);
    return 0;
}
