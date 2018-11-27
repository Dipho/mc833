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

enum state { menu, simple_game, carrasco, multiplayer};

void doit(int connfd, struct sockaddr_in clientaddr);
char *get_word();

int main (int argc, char **argv) {
   int    listenfd,
          connfd,
          port;
   struct sockaddr_in servaddr;
   char   error[MAXDATASIZE + 1];

   if (argc != 2) {
      strcpy(error,"uso: ");
      strcat(error,argv[0]);
      strcat(error," <Port>");
      perror(error);
      exit(1);
   }

   port = atoi(argv[1]);

   listenfd = Socket(AF_INET, SOCK_STREAM, 0);

   servaddr = ServerSockaddrIn(AF_INET, INADDR_ANY, port);

   Bind(listenfd, (struct sockaddr *) &servaddr, sizeof(servaddr));

   Listen(listenfd, LISTENQ);

   for ( ; ; ) {
      pid_t pid;

      struct sockaddr_in clientaddr;
      socklen_t clientaddr_len = sizeof(clientaddr);

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
   char sendline[MAXDATASIZE + 1];

   int n;
   socklen_t remoteaddr_len = sizeof(clientaddr);

   enum state cur_state = menu;

   while ((n = read(connfd, recvline, MAXDATASIZE)) > 0) {
      recvline[n] = 0;

      if (getpeername(connfd, (struct sockaddr *) &clientaddr, &remoteaddr_len) == -1) {
         perror("getpeername() failed");
         return;
      }

      //Esta no Menu
      if (cur_state == menu){

        //SIMPLE GAME REQUEST
        if (recvline[0] == '1'){
          strcpy(sendline, "SIMPLE_GAME");
        }
        //CARRASCO REQUEST
        else if (recvline[0] == '2'){
          strcpy(sendline, "CARRASCO");
        }
        //MULTIPLAYER REQUEST
        else if (recvline[0] == '3'){
          strcpy(sendline, "MULTIPLAYER");
        }
        else {
          strcpy(sendline, "INVALID");
        }
      }

      printf("Linha: %s. Fim da Linha.\n", sendline);
      char* word = get_word();
      printf("Palavra: %s\n", word);
      write(connfd, sendline, strlen(sendline));

   }
}

char *get_word(){
  srand(time(NULL));
  int line_number = ( rand() % 10275 );
  printf("Num: %d\n", line_number);
  FILE *file = fopen("dicionario.txt", "r");
  int count = 0;
  if ( file != NULL ){
    char line[64];
    while (fgets(line, sizeof line, file) != NULL){
        if (count == line_number){
          char* word = malloc(strlen(line));
          strcpy(word, line);
          fclose(file);
          return word;
        }
        else{
          count++;
        }
    }
    fclose(file);
    return NULL;
  }
  else{
    return NULL;
  }
}
