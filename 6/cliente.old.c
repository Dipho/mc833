#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>	/* timeval{} for select() */
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <string.h>
#include <errno.h>
#include <strings.h>
#include <stdlib.h>
#include <unistd.h>

#include "basic.h"
#include "socket_helper.h"

#define MAXLINE 4096
#define EXIT_COMMAND "exit\n"

void doit(int sockfd);
void str_cli(FILE *fp, int sockfd);

int main(int argc, char **argv) {
   int    port, sockfd;
   char * ip;
   char   error[MAXLINE + 1];

   struct sockaddr_in servaddr;

   if (argc != 3) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <IPaddress, Port>");
      perror(error);
      exit(1);
   }

   ip = argv[1];
   port = atoi(argv[2]);

   sockfd = Socket(AF_INET, SOCK_STREAM, 0);

   servaddr = ClientSockaddrIn(AF_INET, ip, port);

   Connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

   // doit(sockfd);
   str_cli(stdin, sockfd);

   Close(sockfd);

   exit(0);
}

void str_cli(FILE *fp, int sockfd) {
	int			maxfdp1, endInput = 0;
	fd_set		rset;
	char		sendline[MAXLINE], recvline[MAXLINE];
  int count = 0;
  //Set é zerado
	FD_ZERO(&rset);
	for ( ; ; ) {

    //Bit correspondente à entrada padrão é setado
		FD_SET(fileno(fp), &rset);
    //Bit correspondente ao socket é setado
		FD_SET(sockfd, &rset);

    //Determina-se qual o maior descriptor de interesse
    if (fileno(fp) > sockfd)
      maxfdp1 = fileno(fp) + 1;
    else
      maxfdp1 = sockfd + 1;

    //Realiza o select
		select(maxfdp1, &rset, NULL, NULL, NULL);

    //Se o socket pode ser lido
		if (FD_ISSET(sockfd, &rset)) {
			if (read(sockfd, recvline, MAXLINE) == 0)
  				perror("str_cli: server terminated prematurely");
			//printf("Linha Recebida: %s. Fim da Linha Recebida.\n", recvline);
      fputs(recvline, stdout);
		}
    //Se a entrada padrão pode ser lida
		else if ((FD_ISSET(fileno(fp), &rset)) && (endInput == 0)) {
      //printf("2\n");
			if (fgets(sendline, MAXLINE, fp) == NULL)
				endInput = 1;
      else{
        //printf("Linha Enviada: %s. Fim da Linha Enviada.\n", sendline);
  			write(sockfd, sendline, strlen(sendline));
      }
		} else {
      if (count == 0){
        usleep(200);
        count++;
      } else
        return;
    }
	}
}
