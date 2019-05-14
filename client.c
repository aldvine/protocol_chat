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
#include <pthread.h>
#include <string.h>
#include <errno.h>

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
    char message[512];
};
typedef struct Client Client;
#define PORT 1024

void *messageServer(void *data);
void viderBuffer();
void LireMessage(Client *c);

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
            //choix du pseudo et de la chaine 
            printf("Saisir votre pseudo:");
            scanf("%s", c.pseudo);
            printf("Saisir la chaine sur laquelle vous voulez diffusez:");
            scanf("%s", c.chanel);
            
            // envoi des infos de channel et pseudo
            strcpy(c.message, "/connect");
            if (send(sock, &c, sizeof(c), 0) == SOCKET_ERROR){
                // TODO faire une cas /connect qui permet de vérifier si le pseudo n'est pas en double s'il existe déja on déconnecte le concerner en lui envoyant un message 
                printf("Erreur de transmission\n");
            }
            // ecoute des messages
            pthread_t thread;
            pthread_create(&thread, NULL, messageServer, (void *)(&sock));
            while (1)
            {
                // boucle sur l'envoi de message
               
                LireMessage(&c);
                if(strcmp(c.message,"/quit")==0){
                    quitServer(sock,c);
                }
                if (send(sock, &c, sizeof(c), 0) == SOCKET_ERROR)
                    printf("Erreur de transmission\n");
            }
        }
        /* sinon, on affiche "Impossible de se connecter" */
        else
        {
            printf("Impossible de se connecter\n");
        }

        /* On ferme la socket */
        close(sock);
    }

    /* On attend que l'utilisateur tape sur une touche, puis on ferme */
    getchar();

    return EXIT_SUCCESS;
}

void viderBuffer()
{
	int c = 0;
	while (c != '\n' && c != EOF)
	{
		c = getchar();
	}
}
void *messageServer(void *socket)
{   
    Client c;
    fd_set readfs;
    int sock = *(int*)socket;
    int statusSocket = 1; // si 0 alors il la socket n'est pas connecté.
    while (statusSocket)
    {
        /* On vide l'ensemble de lecture et on lui ajoute 
                        la socket serveur */
        FD_ZERO(&readfs);
        FD_SET(sock, &readfs);
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

        /* On regarde si la socket client contient des 
                        informations à lire */
        if (FD_ISSET(sock, &readfs))
        {
            statusSocket = recv(sock, &c, sizeof(c), 0);
            if (statusSocket != SOCKET_ERROR)
            {
                printf("%s : %s\n", c.pseudo, c.message);
            }
        }
    }
     close(sock);
}
void LireMessage(Client *c) {
	fgets(c->message, 512, stdin);
	c->message[strlen(c->message) - 1] = '\0';
}

void quitServer(SOCKET s, Client c){
    strcpy(c.message, "/quit");
    send(s, &c, sizeof(c), 0);
    // TODO faire la partie serveur pour passer le client.connecte =0;
    strcpy(c.message, c.pseudo);
    strcpy(c.pseudo,"Serveur");
    strcpy(c.message,  strcat(c.message, " a quitté le salon.\n"));
    printf("debug : %s\n",c.message);
    send(s, &c, sizeof(c), 0); // envoie de l'information aux client du salon
    close(s); // fermeture socket
    return 0;
}
