#include <stdio.h>
#include <stdlib.h>
#include <errno.h>
#include <string.h>
#include <netdb.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <time.h>

#define LISTENQ 10
#define MAXLINE 4096

int Socket(int family, int type, int flags);

void Bind(int sockfd, const struct sockaddr *myaddr, int socklen);

void Listen(int sockfd, int backlog);

int Accept(int sockfd, struct sockaddr *cliaddr, socklen_t *socklen);

void Read(int sockfd, char *recvline, int linelen);

void Write(int sockfd, const void *buf, size_t count);

void Close(int sockfd);

pid_t Fork();

const char *Inet_ntop(int af, const void *src, char *dst, socklen_t size);
//testes
int main (int argc, char **argv) {
   int					listenfd, connfd;
   pid_t				childpid;
   socklen_t			clilen;
   struct sockaddr_in	cliaddr, servaddr;
   void				sig_chld(int);

   int portcli, portserv;
   char ipcli[INET6_ADDRSTRLEN], ipserv[INET6_ADDRSTRLEN];

   char cmd[MAXLINE + 1];
   char recvline[MAXLINE + 1];

   char logc[MAXLINE + 1];
   int recvlen;

   listenfd = Socket(AF_INET, SOCK_STREAM, 0);

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family      = AF_INET;
   servaddr.sin_addr.s_addr = htonl(INADDR_ANY);
   servaddr.sin_port        = htons(atoi(argv[1]));

   Bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
   Listen(listenfd, LISTENQ);

   for ( ; ; ) {
 		clilen = sizeof(cliaddr);
 		connfd = Accept(listenfd, (struct sockaddr *)&cliaddr, &clilen);
    printf("Passou do Accept\n");
 		if ( (childpid = Fork()) == 0) {	/* child process */
 			Close(listenfd);	/* close listening socket */

      /*Obtem o endereço e a porta do cliente conectado ao child*/
      struct sockaddr_in *s = (struct sockaddr_in *)&cliaddr;
      portcli = ntohs(s->sin_port);
      Inet_ntop(AF_INET, &s->sin_addr, ipcli, sizeof (ipcli));;

      /*Log de conexao do cliente*/
      sprintf(logc, "echo `date` : Cliente com IP %s e Porta %u conectou-se >> log.txt", ipcli, portcli);
      system(logc);


      printf("\nIP e Porta Cliente: %s:%u\n", ipcli, portcli);

      /*Obtem o endereço e a porta do servidor*/
      Inet_ntop(AF_INET, &servaddr.sin_addr, ipserv, sizeof(ipserv));
      portserv = ntohs(servaddr.sin_port);
      printf("IP e Porta Servidor: %s:%u\n\n", ipserv, portserv);

      for(;;){
        /*Envia o comando para o cliente*/
        printf("Escreva o comando para o cliente %s:\n", ipcli);
        fgets(cmd, MAXLINE + 1, stdin);
        Write(connfd, cmd, strlen(cmd));

        /*Recebe a resposta do cliente*/
        for (int i=0; i<MAXLINE;i++){
          recvline[i] = '\0';
        }
        Read(connfd, recvline, MAXLINE);
        printf("Resposta do cliente %s: %s", ipcli, recvline);

        /*Log de comando do cliente*/
        recvlen = strlen(recvline);
        recvline[recvlen-1] = '\0';
        sprintf(logc, "echo `date` : Cliente com IP %s e Porta %u executou o comando : %s >> log.txt", ipcli, portcli, recvline);
        system(logc);
        if(strcmp(cmd, "exit\n") == 0)
          break;
      }
      //Read(connfd, recvline, MAXLINE);
      Close(connfd);

      /*Log de desconexao do cliente*/
      sprintf(logc, "echo `date` : Cliente com IP %s e Porta %u desconectou-se >> log.txt", ipcli, portcli);
      system(logc);

 			exit(0);
 		}
 		Close(connfd);			/* parent closes connected socket */
 	}

   return(0);
}

int Socket(int family, int type, int flags){
  int sockfd;

  if ((sockfd = socket(family, type, flags)) < 0) {
	   perror("socket");
	   exit(1);
  } else
    return sockfd;
}

void Bind(int sockfd, const struct sockaddr *myaddr, int socklen){
  if (bind(sockfd, myaddr, socklen) == -1) {
     perror("bind");
     exit(1);
  }
}

void Listen(int sockfd, int backlog){
  if (listen(sockfd, backlog) == -1) {
     perror("listen");
     exit(1);
  }
}

int Accept(int sockfd, struct sockaddr *cliaddr, socklen_t *socklen){
  int accfd;

  if ( (accfd = accept(sockfd, cliaddr, socklen)) == -1) {
    perror("accept error");
    exit(1);
  }

  return accfd;
}

void Read(int sockfd, char *recvline, int linelen){
  int n;

  if ((n = read(sockfd, recvline, linelen)) < 0){
    perror("read");
    exit(1);
  }
}

void Write(int sockfd, const void *buf, size_t count){
  if (write(sockfd, buf, count) == -1){
    perror("write");
    exit(1);
  }
}

void Close(int sockfd){
  if (close(sockfd) == -1){
    perror("close");
    exit(1);
  }
}

pid_t Fork(){
  pid_t childpid;

  if ((childpid = fork()) == -1){
    perror("fork");
    exit(1);
  }
  return childpid;
}

const char *Inet_ntop(int af, const void *src, char *dst, socklen_t size){
  const char *s;

  s = inet_ntop(af, src, dst, size);
  if (s == NULL){
    perror("inet_ntop");
    exit(1);
  } else
    return s;
}
