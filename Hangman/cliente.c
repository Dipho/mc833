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

#include <ncurses.h>

#define MAXLINE 4096
#define EXIT_COMMAND "exit\n"

void init_scr();
void game_menu();
void simple_game();
void game (int lives, char *word, char *exclude);
void print_letters(char *exclude);
void print_spaces(int n);
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

   init_scr();
   game_menu();
   str_cli(stdin, sockfd);

   Close(sockfd);

   exit(0);
}

void init_scr(){
  int maxX;
  int maxY;
  int spaces;

  char titulo[5][107];
  strcpy(titulo[0], "*******  *******  *******  *******      *****      ***        *******  *******  *****    *******    ***  \n");
  strcpy(titulo[1], "   *     *     *  *        *     *      *    *    *   *       *        *     *  *    *   *         *   * \n");
  strcpy(titulo[2], "   *     *     *  *    **  *     *      *     *  * *** *      *****    *     *  *****    *        * *** *\n");
  strcpy(titulo[3], "*  *     *     *  *     *  *     *      *    *   *     *      *        *     *  *    *   *        *     *\n");
  strcpy(titulo[4], " **      *******  *******  *******      *****    *     *      *        *******  *     *  *******  *     *\n");

	initscr();			/* Start curses mode 		*/
  getmaxyx(stdscr, maxY, maxX);
  endwin();

  for (int i=0; i<maxX; i++){
    printf("=");
  }
  printf("\n");
  if (maxX>=105){
    spaces = (maxX-105)/2 - 1;

    print_spaces(spaces);
    printf("%s",titulo[0]);

    print_spaces(spaces);
    printf("%s",titulo[1]);

    print_spaces(spaces);
    printf("%s",titulo[2]);

    print_spaces(spaces);
    printf("%s",titulo[3]);

    print_spaces(spaces);
    printf("%s",titulo[4]);

    for (int i=0; i<maxX; i++){
      printf("=");
    }
    printf("\n");
  }
  else {
    spaces = (maxX-13)/2 - 1;

    print_spaces(spaces);
    printf("JOGO DA FORCA\n");
    for (int i=0; i<maxX; i++){
      mvaddch(2,i,'=');
    }

    for (int i=0; i<maxX; i++){
      printf("=");
    }
    printf("\n");
  }

	return;
}

void game_menu(){
  printf("\nBem vindo ao Jogo da Forca!\n");
  printf("-------\n");
  printf("\n");
  printf("1) Iniciar partida simples.\n");
  printf("2) Ser Carrasco ao iniciar partida.\n");
  printf("3) Jogar no modo multiplayer;\n");
  printf("\n");

  return;
}

void simple_game(){
  printf("\nVocê começou uma partida simples!\n");
  printf("-------\n");

  return;
}

void game (int lives, char *word, char *exclude){
  printf("\n");
  printf("Vidas: %d\n", lives);
  printf("\n");
  printf("Palavra: %s\n", word);
  printf("\n");
  print_letters(exclude);
  return;
}

void print_letters(char *exclude){
  int e_i = 0;
  int abc_i = 0;

  char abc[] = "ABCDEFGHIJKLMNOPQRSTUVWXYZ ";
  int abc_len = 27;

  printf("+-----------------------------------+\n");

  while (abc_i < abc_len){
    if ((abc_i)%9 == 0){
      printf("|");
    }
    if (abc[abc_i] != exclude[e_i]){
      printf(" %c ", abc[abc_i]);
    } else{
      printf("   ");
      e_i++;
    }
    printf("|");
    if ((abc_i + 1)%9 == 0){
      printf("\n");
    }
    abc_i++;
  }
  printf("+-----------------------------------+\n");

  return;
}
void print_spaces(int n){
  for (int i=0; i<n; i++){
    printf(" ");
  }

  return;
}
void str_cli(FILE *fp, int sockfd) {
	int			maxfdp1, send = 0, n;
	fd_set		rset;
	char		sendline[MAXLINE], recvline[MAXLINE] = "MENU";

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

    //SOCKET
		if (FD_ISSET(sockfd, &rset)) {
			if ((n = read(sockfd, recvline, MAXLINE)) == 0)
  				perror("str_cli: server terminated prematurely");
      recvline[n] = 0;

      send = 0;
		}


    //ENTRADA PADRÃO
		if ((FD_ISSET(fileno(fp), &rset)) && (send == 0)) {
      //printf("2\n");
			if (fgets(sendline, MAXLINE, fp) == NULL)
				return;
      else{
        send = 1;
  			write(sockfd, sendline, strlen(sendline));
      }
		}

    if (!strcmp(recvline, "MENU")){
      init_scr();
      game_menu();
    }
    else if (!strcmp(recvline, "SIMPLE_GAME")){
      simple_game();
      game(6, "_ _ _ _ _ _", "AGMPTVWXYZ");
    }
    else if (!strcmp(recvline, "CARRASCO")){
      printf("\nCarrasco a ser implementado\n");
    }
    else if (!strcmp(recvline, "MULTIPLAYER")){
      printf("\nMultiplayer a ser implementado\n");
    }

	}
}
