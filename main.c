#include <arpa/inet.h>
#include <errno.h>
#include <stdio.h>
#include <sys/socket.h>

#define PORT 8080

int main() {
  // Création du socket

  // Paramètres de la fonction : famille d'adresses, type de socket, protocole ip
  int sockfd = socket(AF_INET, SOCK_STREAM, 0);

  if (sockfd == -1) {
    // afficher ce message et le message d'erreur du système avec perror
    perror("webserver (socket)");

    return 1;
  }
  printf("socket created successfully\n");

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
  printf("socket successfully bound to address\n");

  return 0;
}
