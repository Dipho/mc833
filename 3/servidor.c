#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <time.h>
#include <unistd.h>

#define LISTENQ 10
#define MAXDATASIZE 100
#define MAXLINE 4096

int main (int argc, char **argv) {
   int    listenfd, connfd;
   struct sockaddr_in servaddr;
   char   buf[MAXDATASIZE];
   time_t ticks;

   //Alterações exercício 4
   struct sockaddr_in remoteaddr;
   int socket_len = sizeof(remoteaddr);
   char remoteIP[16];
   unsigned int remoteport;

   //Alterações exercício 5
   char recvline[MAXLINE + 1];

   if ((listenfd = socket(AF_INET, SOCK_STREAM, 0)) == -1) {
      perror("socket");
      exit(1);
   }

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family      = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servaddr.sin_port        = htons(8000);

   if (bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1) {
      perror("bind");
      exit(1);
   }

   if (listen(listenfd, LISTENQ) == -1) {
      perror("listen");
      exit(1);
   }

   for ( ; ; ) {
      if ((connfd = accept(listenfd, (struct sockaddr *) NULL, NULL)) == -1 ) {
         perror("accept");
         exit(1);
      }
      //Alterações exercício 4
      /* A função getpeername obtém as informações do socket recebido pelo cli-
      ente
         A função inet_ntop converte o endereço de IP da estrutura do socket de
         binário para texto*/
      getpeername(connfd, (struct sockaddr *) &remoteaddr, &socket_len);
      inet_ntop(AF_INET, &remoteaddr.sin_addr, remoteIP, sizeof(remoteIP));
      remoteport = ntohs(remoteaddr.sin_port);
      printf("IP remoto: %s\n", remoteIP);
      printf("Porta remota: %u\n", remoteport);

      /*Aqui o read recebe o caractere enviado pelo cliente que é enviado de
      volta junto com o timestamp*/
      read(connfd, recvline, MAXLINE);
      ticks = time(NULL);
      snprintf(buf, sizeof(buf), "%.24s\r\n%c\n", ctime(&ticks), recvline[0]);
      write(connfd, buf, strlen(buf));

      /*Este read é feito para ler a mensagem de encerramento de conexão do
      cliente*/
      read(connfd, recvline, MAXLINE);
      close(connfd);
   }
   return(0);
}
