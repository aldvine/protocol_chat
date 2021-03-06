/*-----------------------------------------------------------
Client a lancer apres le serveur avec la commande :
./client
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
    char channel[256];
    char message[512];
};
typedef struct Client Client;
#define PORT 1024

//Liste des fonctions du programme client
void *messageServer(void *data);
void viderBuffer();
void LireMessage(Client *c);
void infoClient(Client *c);
char str_split (char *s, const char *ct);
void quitServer(SOCKET s, Client c, pthread_t thread);

int main(void)
{

    int erreur = 0;
    SOCKET sock;
    
    //Saisie de l'adresse IP du serveur pour la connexion
    char ipServer[15] ;
    printf("Saisir l'adresse IP du serveur format X.X.X.X \n(pour l'adresse local saisissez 'a' puis entrez)): ");
    scanf("%s", ipServer);
    if(strcmp(ipServer,"a")==0){
        strcpy(ipServer,"127.0.0.1");
    }
    SOCKADDR_IN sin;
    // char buffer[32] = "";

    /* Si les sockets Windows fonctionnent */
    if (!erreur)
    {
        int connecte = 0;
        while(!connecte){
             /* Création de la socket */
            sock = socket(AF_INET, SOCK_STREAM, 0);

            /* Configuration de la connexion */
            sin.sin_addr.s_addr = inet_addr(ipServer);
            sin.sin_family = AF_INET;
            sin.sin_port = htons(PORT);
            /* Si l'on a réussi à se connecter */
            if (connect(sock, (SOCKADDR *)&sin, sizeof(sin)) != SOCKET_ERROR)
            {
                connecte =1;
                printf("Connection à %s sur le port %d\n", inet_ntoa(sin.sin_addr), htons(sin.sin_port));
            
                /* Si l'on reçoit des informations : on les affiche à l'écran */
                //declaration du client , il renseigne son nom et la chaine à laquelle il veut se connecter
                Client c;
                //choix du pseudo et de la chaine 
                printf("Saisir votre pseudo:");
                scanf("%s", c.pseudo);
                printf("Saisir la chaine sur laquelle vous voulez diffusez:");
                scanf("%s", c.channel);
                
                //envoi des infos de channel et pseudo
                strcpy(c.message, "/connect");
                if (send(sock, &c, sizeof(c), 0) == SOCKET_ERROR){
                    printf("Erreur de transmission\n");
                }else{
                    printf("Connexion en cours...\n");
                }

                //création d'un thread pour l'écoute des messages envoyés par le serveur
                pthread_t thread;
                pthread_create(&thread, NULL, messageServer, (void *)(&sock));

                //programme qui tourne continuellement et qui permet d'envoyer un message au serveur
                while (1)
                {
                    LireMessage(&c);
                    if(strcmp(c.message,"/quit")==0){
                        
                        quitServer(sock,c,thread);
                        pthread_cancel(thread); // femreture de la socket
                        // close(sock);
                        return 0;
                        break;
                    } else if (strcmp(c.message, "/info")==0){
                        infoClient(&c);
                    } else if(strcmp(c.message, "/channel")==0){
                        printf("Saisir la chaine sur laquelle vous voulez diffusez:");
                        scanf("%s", c.channel);
                        if (send(sock, &c, sizeof(c), 0) == SOCKET_ERROR){
                            printf("Erreur de transmission\n");
                        }
                    } else if (send(sock, &c, sizeof(c), 0) == SOCKET_ERROR){
                        printf("Erreur de transmission\n");
                    }
                }
            }
            /* sinon, on affiche "Impossible de se connecter" */
            else
            {
                connecte=0;
                printf("Impossible de se connecter\n");
                printf("Saisir l'adresse IP du serveur format X.X.X.X \n(pour l'adresse local saisissez 'a' puis entrez)): ");
                scanf("%s", ipServer);
                if(strcmp(ipServer,"a")==0){
                    strcpy(ipServer,"127.0.0.1");
                }
            }   
        }
    }

    /* On attend que l'utilisateur tape sur une touche, puis on ferme */
    getchar();
    return EXIT_SUCCESS;
}

//Vide la saisie du client
void viderBuffer()
{
	int c = 0;
	while (c != '\n' && c != EOF)
	{
		c = getchar();
	}
}

//Thread exécuté dés lors de la connexion au serveur : permet d'écouter les messages envoyés par le serveur et de les afficher
void *messageServer(void *socket)
{   
    Client c;
    fd_set readfs;
    int sock = *(int*)socket;
    int statusSocket = 1; // si 0 alors il la socket n'est pas connecté.
    while (statusSocket !=-1)
    {
        /* On vide l'ensemble de lecture et on lui ajoute la socket serveur */
        FD_ZERO(&readfs);
        FD_SET(sock, &readfs);

        if (select(sock + 1, &readfs, NULL, NULL, NULL) < 0)
        {
            printf("Une erreur est survenu, veuillez contacter votre administraueur\n");
            exit(errno);
        }

        /* On regarde si la socket client contient des informations à lire */
        if (FD_ISSET(sock, &readfs))
        {
            statusSocket = recv(sock, &c, sizeof(c), 0);
            if (statusSocket != SOCKET_ERROR)
            {
                if(strcmp(c.pseudo,"Serveur")==0 && strcmp(c.message,"/full")==0){
                    printf("Le serveur est plein, déconnexion forcée\n");
                    close(sock); // fermeture socket
                    exit(errno);
                } else if(strcmp(c.pseudo,"Serveur")==0 && strcmp(c.message,"/exit")==0){
                    close(sock); // fermeture socket
                    exit(errno);
                } else{
                    printf("%s\n", c.message);
                }
            }
        }
    }
   
}

//Fonction qui permet de lire un message saisi par le client
void LireMessage(Client *c) {
	fgets(c->message, 512, stdin);
	c->message[strlen(c->message) - 1] = '\0';
}

//Fonction qui affiche le menu client
void infoClient(Client *c){
    printf("---------------------Informations----------------------\n");
    printf("Vous êtes sur le channel : %s \n", c->channel);
    printf("Votre pseudo est : %s \n", c->pseudo);
    printf("------------------Liste des commandes------------------\n");
    printf("/quit         - Déconnexion\n");
    printf("/info         - Information chat\n");
    printf("/liste        - Liste des personnes du chat\n");
    printf("/channel      - Changer de channel\n");
    printf("/listechannel - Liste des channels du chat\n");
    printf("----------------Fin liste des commandes----------------\n");
}

//Fonction qui permet de déconnecter le client au serveur
void quitServer(SOCKET s, Client c, pthread_t thread){
    strcpy(c.message, "/quit");
    if(send(s, &c, sizeof(c), 0) != SOCKET_ERROR){
        printf("Message quitter transmis\n");
    } else {
        printf("Erreur transmission\n");
    }
    close(s); // fermeture socket
}
