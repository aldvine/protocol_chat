/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
client <adresse-serveur> <message-a-transmettre>
------------------------------------------------------------*/
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(s) close(s)

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

    int erreur = 0;

    SOCKET sock;
    SOCKADDR_IN sin;
    // char buffer[32] = "";

    /* Si les sockets Windows fonctionnent */
    if (!erreur)
    {
        /* Création de la socket */
        sock = socket(AF_INET, SOCK_STREAM, 0);

        /* Configuration de la connexion */
        sin.sin_addr.s_addr = inet_addr("127.0.0.1");
        sin.sin_family = AF_INET;
        sin.sin_port = htons(PORT);

        /* Si l'on a réussi à se connecter */
        if (connect(sock, (SOCKADDR *)&sin, sizeof(sin)) != SOCKET_ERROR)
        {
            printf("Connection à %s sur le port %d\n", inet_ntoa(sin.sin_addr), htons(sin.sin_port));

            /* Si l'on reçoit des informations : on les affiche à l'écran */
            // declaration du client , il renseigne son nom et la chaine à laquelle il veut se connecter
            Client c;

            printf("Saisir votre pseudo:");
            scanf("%s", c.pseudo);
            printf("Saisir la chaine:");
            scanf("%s", c.chanel);
            int i = 0;
            while (i < 10)
            {
                if (send(sock, &c, sizeof(c), 0) != SOCKET_ERROR)
                    printf("Chaine envoyée : %s\n", c.pseudo);
                else
                    printf("Erreur de transmission\n");
                i++;
            }

            // if (recv(sock, buffer, 32, 0) != SOCKET_ERROR)
            //     printf("Recu : %s\n", buffer);
        }
        /* sinon, on affiche "Impossible de se connecter" */
        else
        {
            printf("Impossible de se connecter\n");
        }

        /* On ferme la socket */
        closesocket(sock);
    }

    /* On attend que l'utilisateur tape sur une touche, puis on ferme */
    getchar();

    return EXIT_SUCCESS;
}