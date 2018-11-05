#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <time.h>
#include <strings.h>
#include <arpa/inet.h>
#include <sys/wait.h>

#include "basic.h"
#include "socket_helper.h"

#define LISTENQ 10
#define MAXDATASIZE 4096
#define EXIT_COMMAND "exit\n"

void doit(int connfd, struct sockaddr_in clientaddr);

int main (int argc, char **argv) {
   int    listenfd,
          connfd,
          port;
   struct sockaddr_in servaddr;
   char   error[MAXDATASIZE + 1];

   // Modificação questao 3
   int backlog;

   if (argc != 3) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <Port>");
      strcat(error," <backlog>");
      perror(error);
      exit(1);
   }

   port = atoi(argv[1]);

   listenfd = Socket(AF_INET, SOCK_STREAM, 0);


   servaddr = ServerSockaddrIn(AF_INET, INADDR_ANY, port);


   Bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

   // Modificação questão 3
   backlog = strtol(argv[2], NULL, 10);
   printf("Backlog: %d\n", backlog);
   Listen(listenfd, backlog);

   for ( ; ; ) {
      pid_t pid;

      struct sockaddr_in clientaddr;
      socklen_t clientaddr_len = sizeof(clientaddr);

      //Sleep necessário para questao 3 e 4
      //sleep(100);
      connfd = Accept(listenfd, (struct sockaddr *) &clientaddr, &clientaddr_len);

      if((pid = fork()) == 0) {
         Close(listenfd);

         doit(connfd, clientaddr);

         Close(connfd);

         exit(0);
      }else{
        waitpid(-1, NULL, 0);
      }

      Close(connfd);
   }

   return(0);
}

void doit(int connfd, struct sockaddr_in clientaddr) {
   char recvline[MAXDATASIZE + 1];
   // char   message[MAXDATASIZE + 1];
   int n;
   socklen_t remoteaddr_len = sizeof(clientaddr);

   while ((n = read(connfd, recvline, MAXDATASIZE)) > 0) {
      recvline[n] = 0;

      // if(strcmp(recvline, EXIT_COMMAND) == 0) {
      //    read(connfd, recvline, MAXDATASIZE);
      //    break;
      // }

      if (getpeername(connfd, (struct sockaddr *) &clientaddr, &remoteaddr_len) == -1) {
         perror("getpeername() failed");
         return;
      }

      //Modificação questao 2
      // printf("<%s-%d>\n", inet_ntoa(clientaddr.sin_addr),(int) ntohs(clientaddr.sin_port));

      // printf("Digite uma mensagem:\n");
      // fgets(message, MAXDATASIZE, stdin);
      // if(strcmp(message, EXIT_COMMAND) == 0) {
      //    break;
      // }
      printf("Linha: %s. Fim da Linha.\n", recvline);
      write(connfd, recvline, strlen(recvline));

   }
}
