#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <stdio.h>
#include <netdb.h>
#include <errno.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <stddef.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>

#define MAXLINE 4096

int my_system (const char *command);

int Socket(int family, int type, int flags);

void Connect(int sockfd, const struct sockaddr *servaddr, int addrlen);

void Read(int sockfd, char *recvline, int linelen);

void Write(int sockfd, const void *buf, size_t count);

void Close(int sockfd);

void Inet_pton(int af, const char *src, void *dst);

const char *Inet_ntop(int af, const void *src, char *dst, socklen_t size);

int main(int argc, char **argv) {
   int    sockfd;
   struct sockaddr_in servaddr;
   char error[MAXLINE + 1];

   int portserv;
   char ipserv[INET_ADDRSTRLEN];

   char recvline[MAXLINE + 1];
   const char sys_erro[] = "Comando inválido\n";

   if (argc != 3) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <IPaddress>");
      perror(error);
      exit(1);
   }

   sockfd = Socket(AF_INET, SOCK_STREAM, 0);

   bzero(&servaddr, sizeof(servaddr));
   servaddr.sin_family = AF_INET;
   servaddr.sin_port   = htons(atoi(argv[2]));
   Inet_pton(AF_INET, argv[1], &servaddr.sin_addr);

   Connect(sockfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

   /*Obtem o endereço e a porta do servidor*/
   struct sockaddr_in *s = (struct sockaddr_in *)&servaddr;
   portserv = ntohs(s->sin_port);
   Inet_ntop(AF_INET, &s->sin_addr, ipserv, sizeof (ipserv));

   printf("IP e Porta Servidor: %s:%u\n\n", ipserv, portserv);
   for(;;){

     for (int i=0; i<MAXLINE;i++){
       recvline[i] = '\0';
     }

     Read(sockfd, recvline, MAXLINE);

     if(strcmp(recvline, "exit\n") == 0){
       Write(sockfd, recvline, strlen(recvline));
       break;
     }

     if (my_system(recvline) != 0){
       Write(sockfd, sys_erro, strlen(sys_erro));
     }else
       Write(sockfd, recvline, strlen(recvline));
   }
   Close(sockfd);

   exit(0);
}


#define SHELL "/bin/sh"

int my_system (const char *command){
  int status = 0;
  pid_t pid;

  pid = fork ();
  if (pid == 0){
      execl (SHELL, SHELL, "-c", command, NULL);
      _exit (EXIT_FAILURE);
    }
  else if (pid < 0)
    status = -1;
  else
    if (waitpid (pid, &status, 0) != pid)
      status = -1;
  return status;
}

int Socket(int family, int type, int flags){
  int sockfd;

  if ((sockfd = socket(family, type, flags)) < 0) {
	   perror("socket");
	   exit(1);
  } else
    return sockfd;
}

void Connect(int sockfd, const struct sockaddr *servaddr, int addrlen){

  if (connect(sockfd, servaddr, addrlen) < 0){
    perror("connect");
    exit(1);
  }

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

void Inet_pton(int af, const char *src, void *dst){
  int s;

  s = inet_pton(af, src, dst);
  if (s <= 0) {
    if (s == 0)
       fprintf(stderr, "Not in presentation format");
    else
       perror("inet_pton");
    exit(1);
  }

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
