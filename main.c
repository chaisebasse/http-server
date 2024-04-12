#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 2048

int main() {
  // Création du buffer
  char buffer[BUFFER_SIZE];

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

  // Déclaration et initialisation de l'adresse
  struct sockaddr_in host_addr;
  host_addr.sin_family = AF_INET;
  // Convertir l'ordre des octets de l'hôte à l'ordre des octets du réseau
  host_addr.sin_port = htons(PORT);
  // Conversion de l'ordre des octets 1a celui du réseau recommandée par man 7 ip mais pas obligatoire
  host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

  int host_addrlen = sizeof(host_addr);

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

  // Boucle continue pour continuer à accepter des nouvelles connections
  for (;;) {
    // Paramètres : socket, adresse du socket avec transtypage, taille de l'adresse avec transtypage
    int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
    if (newsockfd < 0) {
      perror("webserver (accept)");
      continue;
    }

    printf("Connection acceptée\n");

    int valread = read(newsockfd, buffer, BUFFER_SIZE);
    if (valread < 0) {
      perror("webserver (read)");
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
