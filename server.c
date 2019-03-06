#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

void *connectionClient(void *data);
void *messageClient(void *data);

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
    char message[256];
};
typedef struct Client Client;
#define PORT 1024
SOCKET csock;

int list_c[10];
int i = 0;
SOCKET ssock;
int main(void)
{
    /* Socket et contexte d'adressage du serveur */

    SOCKADDR_IN sin;

    socklen_t recsize = sizeof(sin);
    int sock_err;

    /* Socket et contexte d'adressage du client */
    SOCKADDR_IN csin;

    socklen_t crecsize = sizeof(csin);

    /* Création d'une socket */
    ssock = socket(AF_INET, SOCK_STREAM, 0);

    /* Si la socket est valide */
    if (ssock != INVALID_SOCKET)
    {
        printf("La socket %d est maintenant ouverte en mode TCP/IP\n", ssock);

        /* Configuration */
        sin.sin_addr.s_addr = htonl(INADDR_ANY); /* Adresse IP automatique */

        sin.sin_family = AF_INET;   /* Protocole familial (IP) */
        sin.sin_port = htons(PORT); /* Listage du port */
        sock_err = bind(ssock, (SOCKADDR *)&sin, recsize);

        /* Si la socket fonctionne */
        if (sock_err != SOCKET_ERROR)
        {
            /* Démarrage du listage (mode server) */
            sock_err = listen(ssock, 5);
            printf("Listage du port %d...\n", PORT);

            /* Si la socket fonctionne */
            if (sock_err != SOCKET_ERROR)
            {
                // écoute des connections clientes
                while (1)
                {
                    SOCKADDR_IN csin;
                    int crecsize = sizeof csin;
                    printf("Patientez pendant que le client se connecte sur le port %d...\n", PORT);

                    SOCKET csock = accept(ssock, (SOCKADDR *)&csin, &crecsize);
                    i++;
                    list_c[i] = csock; // ajout de la socket dans un tableau
                    printf("Un client se connecte avec la socket %d de %s:%d\n", csock, inet_ntoa(csin.sin_addr), htons(csin.sin_port));
                    pthread_t thread;
                    pthread_create(&thread, NULL, messageClient, (void *)(&csock));
                }
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
        closesocket(ssock);
        printf("Fermeture du serveur terminée\n");
    }
    else
        perror("socket");

    return EXIT_SUCCESS;
}

// récuperation d'un message client
void *messageClient(void *socket)
{   
    fd_set readfs;
    Client c;
    int sock = *(int*)socket; // récuperation de la socket client passé en parametre 
    while (1)
    {
        /* On vide l'ensemble de lecture et on lui ajoute 
                        la socket serveur */
        FD_ZERO(&readfs); // mise à zero lecture;
        FD_SET(sock, &readfs); // récuperation des lecture
        // for (int j = 0; j <= i; j++)
        // {
        //     printf("user  %d:\n", list_c[j]);
        // }
        /* Si une erreur est survenue au niveau du select */
        if (select(sock + 1, &readfs, NULL, NULL, NULL) < 0)
        {
            perror("select()");
            exit(errno);
        }

        /* On regarde si la socket serveur contient des 
                        informations à lire */
        if (FD_ISSET(sock, &readfs))
        {
            // lecture du message recu
            if (recv(sock, &c, sizeof(c), 0) != SOCKET_ERROR)
            {
                printf("%s : %s\n", c.pseudo, c.message);
                if (send(sock, &c, sizeof(c), 0) != SOCKET_ERROR)
                    printf("message envoyé : %s\n", c.message);
                else
                    printf("Erreur de transmission\n");
            }
        }
    }
     close(sock);
}

// faire la fonction permettant d'envoyé un message au bon client en utilisant la socket serveur (var global)
void sendMessage(){
 // TODO
}