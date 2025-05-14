#include <openssl/ssl.h>
#include <openssl/err.h>
#include <netinet/in.h>
#include <unistd.h>
#include <string.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

#define PORT 2222

void init_openssl() {
    SSL_load_error_strings();
    OpenSSL_add_ssl_algorithms();
}

SSL_CTX *create_context() {
    const SSL_METHOD *method = TLS_server_method();
    SSL_CTX *ctx = SSL_CTX_new(method);
    if (!ctx) {
        perror("Unable to create SSL context");
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    SSL_CTX_set_options(ctx, SSL_OP_NO_SSLv2 | SSL_OP_NO_SSLv3);
    SSL_CTX_set_cipher_list(ctx, "HIGH:!aNULL:!MD5");

    return ctx;
}

void configure_context(SSL_CTX *ctx) {
    if (SSL_CTX_use_certificate_file(ctx, "cert/server.crt", SSL_FILETYPE_PEM) <= 0 ||
        SSL_CTX_use_PrivateKey_file(ctx, "cert/server.key", SSL_FILETYPE_PEM) <= 0) {
        ERR_print_errors_fp(stderr);
        exit(EXIT_FAILURE);
    }

    SSL_CTX_load_verify_locations(ctx, "cert/ca.crt", NULL);
    SSL_CTX_set_verify(ctx, SSL_VERIFY_PEER, NULL);
    SSL_CTX_set_verify_depth(ctx, 1);
}

void sauvegarder_resultats(const char *outil, const char *resultat_brut) {
    FILE *f = fopen("rapport_vulnérabilités.txt", "a");
    if (!f) {
        perror("Erreur d'ouverture du fichier de rapport");
        return;
    }

    time_t now = time(NULL);
    fprintf(f, "\n--- Résultat %s (%s) ---\n", outil, ctime(&now));
    fprintf(f, "%s\n", resultat_brut);
    fprintf(f, "--- Fin résultat %s ---\n\n", outil);
    fclose(f);
}

int main() {
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

    printf("Serveur en attente sur le port %d...\n", PORT);

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
                int choix = 0;
                printf("1: Scan (nmap/nikto)\n2: Commande shell libre\n3: Synthèse du scan\n4: Quitter\nVotre choix : ");
                scanf("%d%*c", &choix); // %*c pour enlever le \n

                if (choix == 1) {
                    char tool[64];
                    char target[256];
                    char options[512];
                    char command[1024] = {0};

                    printf("Choisir un outil (nmap, nikto, etc., ou 'exit' pour quitter) : ");
                    if (!fgets(tool, sizeof(tool), stdin)) break;
                    tool[strcspn(tool, "\n")] = 0;

                    if (strcmp(tool, "exit") == 0) {
                        printf("Retour au menu principal.\n");
                        continue;  
                    }

                    printf("Cible (ex: 192.168.100.2 ou http://site.com) : ");
                    if (!fgets(target, sizeof(target), stdin)) break;
                    target[strcspn(target, "\n")] = 0;

                    printf("Options (ex: -sV pour Nmap, -Tuning 9 pour Nikto) : ");
                    if (!fgets(options, sizeof(options), stdin)) break;
                    options[strcspn(options, "\n")] = 0;

                    // Construction dynamique de la commande selon l'outil
                    if (strcmp(tool, "nmap") == 0) {
                        snprintf(command, sizeof(command), "nmap %s %s", options, target);
                    } else if (strcmp(tool, "nikto") == 0) {
                        snprintf(command, sizeof(command), "perl /home/agent1/Bureau/nikto/program/nikto.pl %s -h %s", options, target);
                    } else if (strcmp(tool, "zap") == 0) {
                        snprintf(command, sizeof(command), "zap.sh %s -target %s", options, target);
                    
                    } else {
                        fprintf(stderr, "Outil non reconnu.\n");
                        continue;
                    }

                    // Envoi de la commande
                    SSL_write(ssl, command, strlen(command));

                    // Lecture des résultats
                    printf("Sortie de l'agent :\n");
                    char result_brut[8192] = {0};

                    while (1) {
                        memset(buffer, 0, sizeof(buffer));
                        int bytes = SSL_read(ssl, buffer, sizeof(buffer) - 1);
                        if (bytes <= 0)
                            break;

                        buffer[bytes] = '\0';
                        if (strcmp(buffer, "<END>") == 0)
                            break;

                        strcat(result_brut, buffer);
                        printf("%s", buffer);
                    }

                    sauvegarder_resultats(tool, result_brut);
                    printf("\nCommande terminée.\n");


                    printf("\nCommande terminée.\n");
                } else if (choix == 2) {
                    char command[1024] = {0};
                    printf("Entrez la commande à exécuter sur l'agent (ou 'exit' pour quitter) : ");
                    if (!fgets(command, sizeof(command), stdin))
                    {
                        perror("fgets failed");
                        break;
                    }
    
                    command[strcspn(command, "\n")] = 0;
                    SSL_write(ssl, command, strlen(command));
    
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
                } else if (choix == 3) {
                    FILE *f = fopen("rapport_vulnérabilités.txt", "r");
                    if (!f) {
                        perror("Impossible d’ouvrir le rapport");
                        continue;
                    }
                    char ligne[512];
                    printf("======= Synthèse =======\n");
                    while (fgets(ligne, sizeof(ligne), f)) {
                        printf("%s", ligne);
                    }
                    fclose(f);
                    printf("======= Fin de synthèse =======\n");   
                } else if (choix == 4) {
                    char exit_signal[] = "exit";
                    SSL_write(ssl, exit_signal, strlen(exit_signal));
                    SSL_shutdown(ssl);
                    SSL_free(ssl);
                    close(client);
                    break;

                } else {
                    fprintf(stderr, "Choix invalide.\n");
                }
            }            
        }
    }

    close(sock);
    SSL_CTX_free(ctx);
    EVP_cleanup();
    return 0;
}

