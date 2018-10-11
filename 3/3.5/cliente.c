#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>

#define MAXLINE 4096

int main(int argc, char **argv) {
   int    sockfd, n;
   char   recvline[MAXLINE + 1];
   char   error[MAXLINE + 1];
   struct sockaddr_in servaddr;

   //Alterações exercício 4
   struct sockaddr_in localaddr;
   int socket_len = sizeof(localaddr);
   char localIP[16];
   unsigned int localport;

   //Alterações exercício 5
   char buf[MAXLINE + 1];

   if (argc != 2) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <IPaddress>");
      perror(error);
      exit(1);
   }

   sockfd = socket(AF_INET, SOCK_STREAM, 0);

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port   = htons(8000);

   inet_pton(AF_INET, argv[1], &servaddr.sin_addr);
   connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

   //Alterações exercício 4
   getsockname(sockfd, (struct sockaddr *) &localaddr, &socket_len);
   inet_ntop(AF_INET, &localaddr.sin_addr, localIP, sizeof(localIP));
   localport = ntohs(localaddr.sin_port);
   printf("IP local: %s\n", localIP);
   printf("Porta local: %u\n", localport);

   //Alterações exercício 5
   snprintf(buf, sizeof(buf), "O\r\n");
   write(sockfd, buf, strlen(buf));

   while ( (n = read(sockfd, recvline, MAXLINE)) > 0) {
      recvline[n] = 0;
      close(sockfd);
      if (fputs(recvline, stdout) == EOF) {
         exit(1);
      }
   }
   if (n < 0) {
      exit(1);
   }

   exit(0);
}
