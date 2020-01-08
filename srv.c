/* servTCPIt.c - Exemplu de server TCP iterativ
   Asteapta un nume de la clienti; intoarce clientului sirul
   "Hello nume".
   
   Autor: Lenuta Alboaie  <adria@infoiasi.ro> (c)2009
*/

#include <sys/types.h>

#include <sys/socket.h>

#include <netinet/in.h>

#include <errno.h>

#include <unistd.h>

#include <stdio.h>

#include <string.h>

#include <stdlib.h>

/* portul folosit */
#define PORT 8082
# define MAXDAT 1024
# define ERROR - 1
/* codul de eroare returnat de anumite apeluri */
extern int errno;



struct htmlReq {
      char full[MAXDAT], url[MAXDAT], version[MAXDAT], method[MAXDAT];
};

int main() {
      struct sockaddr_in server; // structura folosita de server
      struct sockaddr_in from;
      int sd; //descriptorul de socket 

      /* crearea unui socket */
      if ((sd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
            perror("[server]Eroare la socket().\n");
            return errno;
      }

      /* pregatirea structurilor de date */
      bzero( & server, sizeof(server));
      bzero( & from, sizeof(from));

      /* umplem structura folosita de server */
      /* stabilirea familiei de socket-uri */
      server.sin_family = AF_INET;
      /* acceptam orice adresa */
      server.sin_addr.s_addr = htonl(INADDR_ANY);
      /* utilizam un port utilizator */
      server.sin_port = htons(PORT);

      /* atasam socketul */
      if (bind(sd, (struct sockaddr * ) & server, sizeof(struct sockaddr)) == -1) {
            perror("[server]Eroare la bind().\n");
            return errno;
      }

      /* punem serverul sa asculte daca vin clienti sa se conecteze */
      if (listen(sd, 5) == -1) {
            perror("[server]Eroare la listen().\n");
            return errno;
      }

      /* servim in mod iterativ clientii... */
      while (1) {
            int client;
            int length = sizeof(from);

            printf("The server is on at 127.0.0.1:%d...\n", PORT);
            fflush(stdout);

            /* acceptam un client (stare blocanta pina la realizarea conexiunii) */
            client = accept(sd, (struct sockaddr * ) & from, & length);

            /* eroare la acceptarea conexiunii de la un client */
            if (client < 0) {
                  perror("[server]Eroare la accept().\n");
                  continue;
            }
            fflush(stdout);
            char msg[] = "HTTP/1.1 404 Not Found:\r\n"
            "Content-Type: text/html; charset=UTF-8\r\n\r\n"
            "<!DOCTYPE html><html><head><title>404 Not Found</title>"
            "<body><h1>404 Not Found</h1></body></html>\r\n";

            /* s-a realizat conexiunea, se astepta mesajul */
            int data_length; // lungimea cererii html de la client
            char recv_data[MAXDAT]; // request-ul clientului transformat in string
            struct htmlReq request;
            char page[MAXDAT];
            bzero(page, MAXDAT);
            if ((data_length = recv(client, recv_data, MAXDAT, 0)) < 0) {
                  perror("[SERVER] Eroare la primirea datelor despre client.");
                  exit(ERROR);
            }



            if (data_length) {
                  char * location, * token;
                  // locatia reprezinta locatia actuala la care am ajuns in string, token-ul separa string-ul in cuvinte
                  /* luam intreg request-ul */
                  token = strtok_r((char * ) recv_data, "\n", & location);
                  // request.full = malloc(sizeof(char) * sizeof(token) + 1);
                  strcpy(request.full, token);
                  // printf("[CLIENT] Request Complet: %s\n", request.full);

                  /* separam din request doar metoda utilizata de client (usually GET) */
                  token = strtok_r((char * ) recv_data, " ", & location);
                  // request.method = malloc(strlen(token) + 1);
                  strcpy(request.method, token);
                  // printf("[CLIENT] Cerere tip: %s\n", request.method);

                  /* in acelasi mod luam url-ul (URI) */
                  token = strtok_r(NULL, " ", & location);
                  // request.url = malloc(strlen(token) + 1);
                  strcpy(request.url, token);
                  // printf("[CLIENT] Adresa client: %s\n", request.url);


                  /* si versiunea de HTML ceruta */
                  // request.version = malloc(strlen(location) + 1);
                  strcpy(request.version, location);
                  // printf("[CLIENT] Versiune: %s\n", request.version);

            }

            FILE * file;
            char path[256];
            bzero(path, 256);
            strcpy(path,"./files");
            // strcpy(path, ".");
            if (strcmp("/", request.url) == 0) {
                  strcat(path, "/index.html");
            } else {
                  strcat(path, request.url);
            }
            /*Get extension */
            char * ext;
            char url[100];
            strcpy(url, request.url);
            strtok_r(url, ".", & ext);
            /*
                  Selectam headerul pe baza extensiei
            */
           /*
            .html sau nimic trimitem html header
           */
            if (strcmp(ext, "html") == 0 || strcmp(ext, "") == 0) {
                  char html[] = "HTTP/1.1 200 OK:\r\n"
                  "Content-Type: text/html; charset=UTF-8\r\n\r\n";
                  write(client, html, sizeof(html));
            } else {
                  /*
                  .txt trimitem text/plain header 
                  */ 
                  if (strcmp(ext, "txt") == 0) {
                        char html[] = "HTTP/1.1 200 OK:\r\n"
                        "Content-Type: text/plain; charset=UTF-8\r\n\r\n";
                        write(client, html, sizeof(html));
                  } else {
                        /*
                        altfel trimitem html pentru eroare 
                        */ 
                        char err[] = "HTTP/1.1 404 Not Found:\r\n"
                        "Content-Type: text/html; charset=UTF-8\r\n\r\n";
                        write(client, err, sizeof(err));
                  }
            }
            /*Daca nu are extensie */ 
            if (strcmp(ext, "") == 0 && strcmp(path,"./files/index.html")!=0 ) {
                  strcat(path, ".html");
            }
            //Verify file and read it 
            if (access(path, F_OK) != -1) {
                  file = fopen(path, "r");
                  char text[2048];
                  char ch;
                  bzero(text, 2048);
                  ch = fgetc(file);
                  while (ch != EOF) {
                        strncat(text, & ch, 1);
                        ch = fgetc(file);
                  }
                  fclose(file);
                  write(client, text, sizeof(text));
            } else {
                  // file doesn't exist
                  strcpy(page, "<!DOCTYPE html><html><head><title>404 Not Found</title>"
                        "<body><h1>404 Not Found</h1>"
                        "<a href=\"/\"> Home</a> "
                        "</body></html>\r\n");
                  write(client, page, sizeof(page));
            }

            close(client);
      } /* while */
} /* main */
