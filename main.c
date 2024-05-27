#include <stdio.h>
#include <string.h>
#include <arpa/inet.h>
#include <errno.h>
#include <sys/socket.h>
#include <unistd.h>

#define PORT 8080
#define BUFFER_SIZE 2048

const char *path_page = "./fichiers_site"; // mettre le chemin vers la page html

void serve_file(int client_socket, const char *file_path, const char *file_extension);
void handle_client(int client_socket, struct sockaddr_in client_addr);
const char *get_content_type(const char *file_path);

int main() {
    // Création du socket
    int sockfd = socket(AF_INET, SOCK_STREAM, 0);
    if (sockfd == -1) {
        // afficher ce message et le message d'erreur du système avec perror
        perror("webserver (socket)");
        printf("\n");
        return 1;
    }
    printf("Socket créé avec succès\n");

    // Permettre la réutilisation de l'adresse
    int enable = 1;
    if (setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &enable, sizeof(enable)) < 0) {
        // afficher ce message et le message d'erreur du système avec perror
        perror("webserver (setsockopt)");
        printf("\n");
        return 1;
    }

    // Bind le socket à l'addresse
    struct sockaddr_in host_addr;
    int host_addrlen = sizeof(host_addr);
    host_addr.sin_family = AF_INET;
    host_addr.sin_port = htons(PORT);
    host_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    // Déclaration de l'adresse du client et sa taille
    struct sockaddr_in client_addr;
    int client_addrlen = sizeof(client_addr);

    // Paramètres de la fonction : adresse choisie convertie en type accepté par la fonction, taille de l'adresse
    if (bind(sockfd, (struct sockaddr *)&host_addr, host_addrlen) != 0) {
        // afficher ce message et le message d'erreur du système avec perror
        perror("webserver (bind)");
        printf("\n");
        return 1;
    }
    printf("Socket lié à une addresse avec succès\n");

    // Écouter les transmissions entrantes
    if (listen(sockfd, SOMAXCONN) != 0) {
        // afficher ce message et le message d'erreur du système avec perror
        perror("webserver (listen)");
        printf("\n");
        return 1;
    }
    printf("Serveur en écoute pour des connections\n");
    printf("http://localhost:8080/\n");
    printf("\n");

    // Accepter les connections en attente
    for (;;) {
        // On travaille ici avec le socket accepté
        // Paramètres : socket, adresse du socket avec transtypage, taille de l'adresse avec transtypage
        int newsockfd = accept(sockfd, (struct sockaddr *)&host_addr, (socklen_t *)&host_addrlen);
        if (newsockfd < 0) {
            // afficher ce message et le message d'erreur du système avec perror
            perror("webserver (accept)");
            printf("\n");
            continue;
        }

        printf("Connection acceptée\n");

        // Prendre l'adresse du client

        // Paramètres : socket, adresse du client avec transtypage, taille de l'adresse avec transtypage
        int sockn = getsockname(newsockfd, (struct sockaddr *)&client_addr, (socklen_t *)&client_addrlen);
        if (sockn < 0) {
            // afficher ce message et le message d'erreur du système avec perror
            perror("webserver (getsockname)");
            printf("\n");
            continue;
        }

        // Appel de la fonction gérant le client
        handle_client(newsockfd, client_addr);

        // Fermer la connection à la fin des intéractions
        close(newsockfd);
    }

    return 0;
}

void handle_client(int client_socket, struct sockaddr_in client_addr) {
  // Ce buffer sert a lire les données entrantes du client
  char buffer[BUFFER_SIZE];

  // Lire dans le socket
  int valread = read(client_socket, buffer, BUFFER_SIZE);
  if (valread < 0) {
    // afficher ce message et le message d'erreur du système avec perror
    perror("webserver (read)");
    printf("\n");
    return;
  }

  // Lire la requête

  // On initialise la méthode de la requête (<method>), le chemin (<path>) et la version (<version>)
  char method[BUFFER_SIZE], uri[BUFFER_SIZE], version[BUFFER_SIZE];
  // On lit les trois éléments du buffer
  sscanf(buffer, "%s %s %s", method, uri, version);
  // On affiche la représentation en string du port, de l'adresse IP du client et de ses requêtes
  printf("[%s:%u] %s %s %s\n", inet_ntoa(client_addr.sin_addr), ntohs(client_addr.sin_port), method, uri, version);

  // Déterminer le chemin requêté
  char file_path[BUFFER_SIZE];
  const char *subfolder_path = path_page;

  if (strlen(uri) == 0 || strcmp(uri, "/") == 0) {
    // Gérer le cas où aucun chemin spécifique n'est requêté
    snprintf(file_path, sizeof(file_path), "%s/index.html", subfolder_path);
  } else {
    // Construire le fichier selon le chemin requêté
    // Calculer la taille max de l'URI
    int max_uri_length = sizeof(file_path) - strlen(subfolder_path) - 1;
    // Prendre la taille de l'URI
    int uri_length = strlen(uri);

    // S'assurer que la taille de l'URI n'excède pas la taille max
    if (uri_length > max_uri_length) {
      uri_length = max_uri_length; // Écorcher si nécessaire
    }

    // Copier l'URI dans file_path avec le bon format
    snprintf(file_path, sizeof(file_path), "%s%s", subfolder_path, uri);

    // S'assurer que file_path se termine avec NULL
    file_path[sizeof(file_path) - 1] = '\0';
  }

  // Déterminer l'extension du fichier selon l'URI
  const char *file_extension = strrchr(uri, '.');
  if (file_extension != NULL) {
    file_extension++; // On passe le '.' qui précède le nom de l'extension
  }

  // Appel de la fonction servant le fichier demandé
  serve_file(client_socket, file_path, file_extension);

  // Fermer la connection à la fin des intéractions
  close(client_socket);
}

void serve_file(int client_socket, const char *file_path, const char *file_extension) {
    // Ouvrir le fichier en mode lecture binaire
    printf("%s\n", file_path);
    FILE *file = fopen(file_path, "rb");
    if (file == NULL) {
        // afficher ce message et le message d'erreur du système avec perror
        perror("webserver (fopen)");
        printf("\n");
        // Répondre avec Error 404 si le fichier n'existe pas
        const char *not_found_msg = "HTTP/1.0 404 Not Found\r\n\r\n<html><body><h1>404 Not Found</h1></body></html>\r\n";
        write(client_socket, not_found_msg, strlen(not_found_msg));
        return;
    }

    // Déterminer le type du contenu en fonction de l'extension
    const char *content_type = get_content_type(file_path); // On admet que index.html contient de l'HTML

    // Construire et envoyer le header HTTP avec le type du contenu
    char header[BUFFER_SIZE];
    snprintf(header, sizeof(header), "HTTP/1.0 200 OK\r\nServer: webserver-c\r\nContent-Type: %s\r\n", content_type);
    // Ajouter des en-têtes pour désactiver la mise en cache
    strcat(header, "Cache-Control: no-cache, no-store, must-revalidate\r\n");
    strcat(header, "Pragma: no-cache\r\n");
    strcat(header, "Expires: 0\r\n");
    strcat(header, "\r\n");
    write(client_socket, header, strlen(header));

    // Envoyer le contenu du fichier
    char buffer[BUFFER_SIZE];
    size_t bytes_read;
    while ((bytes_read = fread(buffer, 1, BUFFER_SIZE, file)) > 0) {
        if (write(client_socket, buffer, bytes_read) < 0) {
            // afficher ce message et le message d'erreur du système avec perror
            perror("webserver (write)");
            printf("\n");
            break;
        }
    }

    printf("\n");

    // Fermer le fichier
    fclose(file);
}

const char *get_content_type(const char *file_path) {
    const char *content_type = "application/octet-stream"; // Type de contenu par défaut (données binaires)

    // Trouver la dernière apparition de '.' pour prendre l'extension du fichier
    const char *extension = strrchr(file_path, '.');
    if (extension != NULL) {
        if (strcmp(extension, ".html") == 0 || strcmp(extension, ".htm") == 0) {
            content_type = "text/html";
        } else if (strcmp(extension, ".css") == 0) {
            content_type = "text/css";
        } else if (strcmp(extension, ".js") == 0) {
            content_type = "application/javascript";
        }
    }

    return content_type;
}
