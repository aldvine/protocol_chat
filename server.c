#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(param) close(param)
#define CLIENT_MAXIMUM 200

typedef int SOCKET;
typedef struct sockaddr_in SOCKADDR_IN;
typedef struct sockaddr SOCKADDR;
struct Client
{
    char pseudo[256];
    char chanel[256];
    char message[512];
};
typedef struct Client Client;
struct ClientConfig
{
    Client client;
    SOCKET socket;
    pthread_t thread;
    int connecte; // variable qui sert a savoir si un client est présent dans la case du tableau, si il est pas présent on peut utiliser cette emplacement.
};
typedef struct ClientConfig ClientConfig;

#define PORT 1024
SOCKET csock;

ClientConfig list_c[CLIENT_MAXIMUM];
int nb = 0;
SOCKET ssock;

void *messageClient(void *clientConf);
void sendMessage(Client c);

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

                    ClientConfig clientConf ;
                    SOCKET socket_temp=accept(ssock, (SOCKADDR *)&csin, &crecsize);
                    for (int j = 0; j <= CLIENT_MAXIMUM; j++){
                        if(list_c[j].connecte==0){
                            nb=j;
                            list_c[nb].socket = socket_temp; 
                            list_c[nb].connecte = 1 ;
                            break;
                        }else if(j==CLIENT_MAXIMUM){
                            // sendServerFull(socket);
                            // envoyé messsage serveur plein au client
                        }
                    }
                    printf("Un client se connecte avec la socket %d de %s:%d\n", csock, inet_ntoa(csin.sin_addr), htons(csin.sin_port));
                    pthread_create(&list_c[nb].thread, NULL, messageClient, &list_c[nb]); // utiliser le tableau pour passer le client.
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
void *messageClient(void *clientConf)
{
    fd_set readfs;
    ClientConfig * clientC = (ClientConfig *) clientConf;
    // on travail avec clientC;
  
    int sock = *(int *)socket; // récuperation de la socket client passé en parametre

    while (1)
    {
        /* On vide l'ensemble de lecture et on lui ajoute 
                        la socket serveur */
        FD_ZERO(&readfs);      // mise à zero lecture;
        FD_SET((*clientC).socket, &readfs); // récuperation des lecture
        // for (int j = 0; j <= i; j++)
        // {
        //     printf("user  %d:\n", list_c[j]);
        // }
        /* Si une erreur est survenue au niveau du select */
        if (select((*clientC).socket + 1, &readfs, NULL, NULL, NULL) < 0)
        {
            perror("select()");
            exit(errno);
        }

        /* On regarde si la socket serveur contient des 
                        informations à lire */
        if (FD_ISSET((*clientC).socket, &readfs))
        {
            // lecture du message recu
            if (recv((*clientC).socket, &(*clientC).client, sizeof((*clientC).client), 0) != SOCKET_ERROR)
            {
                printf("%s : %s\n", (*clientC).client.pseudo, (*clientC).client.message);
                sendMessage((*clientC).client);
            }
        }
    }
    close(sock);
}

// faire la fonction permettant d'envoyé un message au bon client en utilisant la socket serveur (var global)
// il faudra probablement utiliser les mutex pour verouiller la liste de sockets client lors de la lecture ou l'ecriture
void sendMessage(Client c)
{
    // for (int i = 0; i < CLIENT_MAXIMUM; i++)
    // {
        // if (list_c[i].c)
        // {
            // if (send(list_c[i], &c, sizeof(c), 0) != SOCKET_ERROR)
            // {
            //     printf("message envoyé : %s\n", c.message);
            // }
            // else
            // {
            //     printf("Erreur de transmission\n");
            // }
        // }
    // }
}
