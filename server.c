#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(param) close(param)

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
struct Client
{
    char pseudo[256];
    char chanel[256];
};
typedef struct Client Client;
#define PORT 1024

int main(void)
{
    /* Socket et contexte d'adressage du serveur */

    SOCKADDR_IN sin;
    SOCKET sock;
    socklen_t recsize = sizeof(sin);

    int sock_err;

    /* Socket et contexte d'adressage du client */
    SOCKADDR_IN csin;
    SOCKET csock;
    socklen_t crecsize = sizeof(csin);

    /* Création d'une socket */
    sock = socket(AF_INET, SOCK_STREAM, 0);

    /* Si la socket est valide */
    if (sock != INVALID_SOCKET)
    {
        printf("La socket %d e./st maintenant ouverte en mode TCP/IP\n", sock);

        /* Configuration */
        sin.sin_addr.s_addr = htonl(INADDR_ANY); /* Adresse IP automatique */

        sin.sin_family = AF_INET;   /* Protocole familial (IP) */
        sin.sin_port = htons(PORT); /* Listage du port */
        sock_err = bind(sock, (SOCKADDR *)&sin, recsize);

        /* Si la socket fonctionne */
        if (sock_err != SOCKET_ERROR)
        {
            /* Démarrage du listage (mode server) */
            sock_err = listen(sock, 5);
            printf("Listage du port %d...\n", PORT);

            /* Si la socket fonctionne */
            if (sock_err != SOCKET_ERROR)
            {
                /* Attente pendant laquelle le client se connecte */
                printf("Patientez pendant que le client se connecte sur le port %d...\n", PORT);
                csock = accept(sock, (SOCKADDR *)&csin, &crecsize);
                printf("Un client se connecte avec la socket %d de %s:%d\n", csock, inet_ntoa(csin.sin_addr), htons(csin.sin_port));
                Client c;

                while (1)
                {
                    if(recv(csock, &c, sizeof(c), 0)!= SOCKET_ERROR){
                        printf("Recu : %s\n", c.pseudo);
                    }
                  
                }
                // if (recv(sock, &c, sizeof(c), 0) != SOCKET_ERROR){

                // char chaine[50] = "Hello world";
                // send(csock, chaine, sizeof(chaine), 0);
                // SOCKET csock2 = accept(sock, (SOCKADDR *)&csin, &crecsize);
            }
            else
                perror("listen");
        }
        else
            perror("bind");

        /* Fermeture de la socket client et de la socket serveur */
        printf("Fermeture de la socket client\n");
        closesocket(csock);
        printf("Fermeture de la socket serveur\n");
        closesocket(sock);
        printf("Fermeture du serveur terminée\n");
    }
    else
        perror("socket");

    return EXIT_SUCCESS;
}