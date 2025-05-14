#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

#define PORT 2222

void init_openssl()
{
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

SSL_CTX *create_context()
{
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx)
    {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
    return ctx;
}

void configure_context(SSL_CTX *ctx)
{
    if (SSL_CTX_use_certificate_file(ctx, "cert/server.crt", SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, "cert/server.key", SSL_FILETYPE_PEM) <= 0)
    {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }
}

int main()
{
    int sock;
    struct sockaddr_in addr;
    char buffer[4096];

    init_openssl();
    SSL_CTX *ctx = create_context();
    configure_context(ctx);

    sock = socket(AF_INET, SOCK_STREAM, 0);
    addr.sin_family = AF_INET;
    addr.sin_port = htons(PORT);
    addr.sin_addr.s_addr = INADDR_ANY;

    bind(sock, (struct sockaddr *)&addr, sizeof(addr));
    listen(sock, 1);

    printf("Server listening on port %d...\n", PORT);

    while (1)
    {
        struct sockaddr_in client_addr;
        unsigned int len = sizeof(client_addr);
        int client = accept(sock, (struct sockaddr *)&client_addr, &len);

        SSL *ssl = SSL_new(ctx);
        SSL_set_fd(ssl, client);

        if (SSL_accept(ssl) <= 0)
        {
            ERR_print_errors_fp(stderr);
        }
        else
        {
            while (1)
            {
                char command[1024] = {0};
                printf("Entrez la commande à exécuter sur l'agent (ou 'exit' pour quitter) : ");
                if (!fgets(command, sizeof(command), stdin))
                {
                    perror("fgets failed");
                    break;
                }

                command[strcspn(command, "\n")] = 0;
                SSL_write(ssl, command, strlen(command));

                if (strcmp(command, "exit") == 0)
                    break;

                printf("Sortie de l'agent :\n");
                while (1)
                {
                    memset(buffer, 0, sizeof(buffer));
                    int bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
                    if (bytes <= 0)
                        break;

                    buffer[bytes] = '\0';
                    if (strcmp(buffer, "<END>") == 0)
                        break;

                    printf("%s", buffer);
                }

                printf("\nCommande terminée.\n");
            }
        }
    }

    close(sock);
    SSL_CTX_free(ctx);
    EVP_cleanup();
    return 0;
}

