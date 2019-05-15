#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <pthread.h>
#include <errno.h>
#include <string.h>
#include <time.h> 

#define INVALID_SOCKET -1
#define SOCKET_ERROR -1
#define closesocket(param) close(param)
#define CLIENT_MAXIMUM 100

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
void sendMessageToDest(ClientConfig cConf);

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

    //****** init tableau des client
        ClientConfig clientConf ;
        for (int j = 0; j < CLIENT_MAXIMUM; j++){
            list_c[j].connecte=0;
        }
    // *********
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
                 SOCKADDR_IN csin;
                int crecsize = sizeof csin;
                printf("Patientez pendant que le client se connecte sur le port %d...\n", PORT);
                ClientConfig clientConf ;
                SOCKET socket_temp;
                while (socket_temp=accept(ssock, (SOCKADDR *)&csin, &crecsize))
                {
                   
                    for (int j = 0; j < CLIENT_MAXIMUM; j++){
                        if(list_c[j].connecte==0){
                            nb=j;
                            list_c[nb].socket = socket_temp; 
                            list_c[nb].connecte = 1 ;
                            printf("Un client se connecte avec la socket %d de %s:%d\n", csock, inet_ntoa(csin.sin_addr), htons(csin.sin_port));
                            pthread_create(&list_c[nb].thread, NULL, messageClient, &list_c[nb]); // utiliser le tableau pour passer le client.
                            break;
                        }else if(j==CLIENT_MAXIMUM-1){
                            clientConf.socket = socket_temp;
                            Client c_temp ;
                            strcpy(c_temp.pseudo, "Serveur");
                            strcpy(c_temp.message, "Complet");
                            clientConf.client = c_temp;
                            sendMessageToDest(clientConf);
                            printf("Rejet d'un client pour cause :serveur plein\n");
                            shutdown(socket_temp,2);
                            // envoyé messsage serveur plein au client
                        }
                    }
                   
                    printf("Patientez pendant que le client se connecte sur le port %d...\n", PORT);
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
    ClientConfig clientConf;
    char tmpPseudo[256];
    char temp[256];
    char date[256]; 
    time_t timestamp = time(NULL); 
  
    strftime(date, sizeof(date), "[%x - %X]", localtime(&timestamp));
    strcat(date, " ");

    if(c.message[0] != '\0'){
        if(strcmp(c.message, "/liste")==0)
        {
            strcpy(c.message, date);
            strcat(c.message, "Liste des pseudos connectés au channel :");
            for(int i = 0; i < CLIENT_MAXIMUM; i++)
            {
                if(&list_c[i].client)
                {
                    if(strcmp(&list_c[i].client.chanel,c.chanel)==0 && list_c[i].connecte == 1)
                    {
                        strcpy(tmpPseudo, "\n");
                        strcat(tmpPseudo, &list_c[i].client.pseudo);
                        strcat(c.message, tmpPseudo);
                        if(strcmp(&list_c[i].client.pseudo,c.pseudo)==0)
                        {
                            clientConf = list_c[i];
                        }
                    }
                }
            }
            if (send(clientConf.socket, &c, sizeof(c), 0) != SOCKET_ERROR)
            {
                printf("%sListe des pseudos dans le chanel %s transmis à %s\n", date, c.chanel,c.pseudo);
            }
            else
            {
                printf("%sErreur de transmission message : %s \n", date, c.message);
            }
        } else if(strcmp(c.message, "/channel")==0) 
        {
            for (int i = 0; i < CLIENT_MAXIMUM; i++)
            {
                if (&list_c[i].client)
                {
                    if (strcmp(&list_c[i].client.pseudo,c.pseudo)== 0 && list_c[i].connecte == 1){
                        list_c[i].client = c;
                        strcpy(c.message, date);
                        strcat(c.message, "Changement de channel OK");
                        if (send(list_c[i].socket, &c, sizeof(c), 0) != SOCKET_ERROR)
                        {
                            printf("%sChangement de channel transmis à %s\n", date, list_c[i].client.pseudo);
                        }
                        else
                        {
                            printf("%sErreur de transmission\n", date);
                        }
                    }
                }
            }
        }else {
            strcpy(temp, c.message);
            for (int i = 0; i < CLIENT_MAXIMUM; i++)
            {
                if (&list_c[i].client)
                {
                    if ((compare(list_c[i].client.chanel,c.chanel)== 1 || strstr(list_c[i].client.chanel, "hack")) && list_c[i].connecte == 1){
                        strcpy(c.message, date);
                        strcat(c.message, c.pseudo);
                        strcat(c.message, " : ");
                        strcat(c.message, temp);
                        if (send(list_c[i].socket, &c, sizeof(c), 0) != SOCKET_ERROR)
                        {
                            printf("%sMessage transmis par %s sur la channel %s\n", date, c.pseudo, c.chanel);
                        }
                        else
                        {
                            printf("%sErreur de transmission\n", date);
                        }
                    }
                }
            }
        }
    }
}

void sendMessageToDest(ClientConfig cConf)
{
    Client c= cConf.client;
    char date[256]; 
    time_t timestamp = time(NULL); 
  
    strftime(date, sizeof(date), "[%x - %X]", localtime(&timestamp));
    strcat(date, " ");

    if (send(cConf.socket, &c, sizeof(c), 0) != SOCKET_ERROR)
    {
        printf("%sMessage transmis a %s\n", date, c.pseudo);
    }
    else
    {
        printf("%sErreur de transmission\n", date);
    }
}

int compare(const char* chaine1, const char* chaine2)
{   unsigned int i=0;
    if( strlen(chaine1) != strlen(chaine2) )
        return 0;
    for(i=0;i<strlen(chaine1);i++)
        if( chaine1[i] != chaine2[i])
            return 0;
    return 1;
}
