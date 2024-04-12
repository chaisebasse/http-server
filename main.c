#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 2048

int main() {
  // Création du buffer
  char buffer[BUFFER_SIZE];
  char resp[] = "HTTP/1.0 200 OK\r\n"
                "Server: webserver-c\r\n"
                "Content-type: text/html\r\n\r\n"
                "<html>hello, world</html>\r\n";

  // Création du socket

  // Paramètres de la fonction : famille d'adresses, type de socket, protocole ip
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);
  if (sockfd == -1) {
    // afficher ce message et le message d'erreur du système avec perror
    perror("webserver (socket)");

    return 1;
  }
  printf("Socket créé avec succès\n");

  // Bind le socket à l'addresse

  // Déclaration et initialisation de l'adresse de l'hôte et sa taille
  struct sockaddr_in host_addr;
  int host_addrlen = sizeof(host_addr);
  host_addr.sin_family = AF_INET;
  // Convertir l'ordre des octets de l'hôte à l'ordre des octets du réseau
  host_addr.sin_port = htons(PORT);
  // Conversion de l'ordre des octets 1a celui du réseau recommandée par man 7 ip mais pas obligatoire
  host_addr.sin_addr.s_addr = htonl(INADDR_ANY);


  // Déclaration de l'adresse du client et sa taille
  struct sockaddr_in client_addr;
  int client_addrlen = sizeof(client_addr);

  // Paramètres de la fonction : adresse choisie convertie en type accepté par la fonction, taille de l'adresse
  if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
    perror("webserver (bind)");
    return 1;
  }
  printf("Socket lié à une addresse avec succès\n");

  // Écouter les transmissions entrantes

  // SOMAXCONN = nombre max de conections en attente à la fois (128 par défaut)
  if (listen(sockfd, SOMAXCONN) != 0) {
    perror("webserver (listen)");
    return 1;
  }
  printf("Serveur en écoute pour des connections\n");

  // Accepter les connections en attente

  // Boucle continue pour continuer à accepter de nouvelles connections
  for (;;) {
    // On travaille ici avec le socket accepté
    // Paramètres : socket, adresse du socket avec transtypage, taille de l'adresse avec transtypage
    int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
    if (newsockfd < 0) {
      perror("webserver (accept)");
      continue;
    }

    printf("Connection acceptée\n");

    // Prendre l'adresse du client

    // Paramètres : socket, adresse du client avec transtypage, taille de l'adresse avec transtypage
    int sockn = getsockname(newsockfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen);
    if (sockn < 0) {
      perror("webserver (getsockname)");
      continue;
    }

    // Lire dans le socket

    // Paramètres : socket, l'endroit où on lit (buffer), taille du buffer
    int valread = read(newsockfd, buffer, BUFFER_SIZE);
    if (valread < 0) {
      perror("webserver (read)");
      continue;
    }


    // On affiche la représentation en string du port et de l'adresse IP du client
    printf("[%s:%u]\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port));

    // Écrire dans le socket

    // Paramètres : socket, ce qu'on écrit, taille de ce qu'on écrit
    int valwrite = write(newsockfd, resp, strlen(resp));
    if (valwrite < 0) {
      perror("webserver (write)");
      continue;
    }


    // Fermer la connection à la fin des intéractions
    close(newsockfd);
  }

  return 0;
}


// Fonction d'intéraction socket-client

// void handle_client(int client_socket) {
//   char buffer[BUFFER_SIZE];

//   int valread = read(client_socket, buffer, BUFFER_SIZE);
//   if (valread < 0) {
//     perror("webserver (read)");
//     continue;
//   }

// }
