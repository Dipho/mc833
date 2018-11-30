#include <sys/socket.h>
#include <sys/types.h>
#include <sys/select.h>
#include <sys/time.h>
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
#define clear_scr() system("clear");

enum state { menu, simple_game, carrasco, multiplayer};

void init_scr();
void game_menu();
void game(int lives, char *word, char *exclude);
void print_letters(char *exclude);
void print_spaces(int n);
void update_used_letters (char new_letter, char *used_letters);
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

// Imprime a tela inicial do jogo
void init_scr(){
  int maxX;
  int spaces;

  char titulo[5][107];
  strcpy(titulo[0], "*******  *******  *******  *******      *****      ***        *******  *******  *****    *******    ***  \n");
  strcpy(titulo[1], "   *     *     *  *        *     *      *    *    *   *       *        *     *  *    *   *         *   * \n");
  strcpy(titulo[2], "   *     *     *  *    **  *     *      *     *  * *** *      *****    *     *  *****    *        * *** *\n");
  strcpy(titulo[3], "*  *     *     *  *     *  *     *      *    *   *     *      *        *     *  *    *   *        *     *\n");
  strcpy(titulo[4], " **      *******  *******  *******      *****    *     *      *        *******  *     *  *******  *     *\n");

  clear_scr();

  // Utiliza curses para obter o tamanho do terminal
	initscr();
  getmaxyx(stdscr, maxX, maxX);
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

// Imprime o menu do jogo
void game_menu(){
  printf("\nBem vindo ao Jogo da Forca!\n");
  printf("-------\n");
  printf("\n");
  printf("1) Iniciar partida simples.\n");
  printf("2) Ser Carrasco ao iniciar partida.\n");
  printf("3) Jogar no modo multiplayer.\n");
  printf("4) Sair do jogo =C\n");
  printf("\n");

  return;
}

// Imprime o core das partidas
void game(int lives, char *word, char *exclude){
  printf("----------------------------------------------------------------------\n");
  printf("Vidas: %d\n", lives);
  printf("\n");
  printf("Palavra: %s\n", word);
  printf("\n");
  print_letters(exclude);
  printf("\n");
  printf("Digite seu palpite ('exit' para cancelar partida): ");
  printf("\n");
  return;
}

// Imprime o alfabeto de letras disponíveis
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

// Auxiliar para a função "print_letters"
void print_spaces(int n){
  for (int i=0; i<n; i++){
    printf(" ");
  }

  return;
}

// Atualiza o array de caracteres utilizados
void update_used_letters (char new_letter, char *used_letters){

  char aux_letters[26];
  strcpy(aux_letters, used_letters);

  int i;
  for (i = 0; i<strlen(aux_letters); i++){
    if (aux_letters[i] == new_letter){
      return;
    }
    else if (aux_letters[i] > new_letter){
      used_letters[i] = new_letter;
      for (int j = i; j<strlen(aux_letters); j++){
        used_letters[j+1] = aux_letters[j];
      }

      return;
    }
  }
  used_letters[i+1] = used_letters[i];
  used_letters[i] = new_letter;

  return;
}

// Realiza comunicação com o servidor
void str_cli(FILE *fp, int sockfd) {
  // Variáveis para comunicação
	int			maxfdp1, send = 0, n;
	fd_set		rset;
	char		sendline[MAXLINE], recvline[MAXLINE] = "MENU";

  // Variáveis da aplicação
  enum state cur_state = menu;

  int j, lives = -1;
  char *cli_word;
  char used_letters[26] = " ";

  //Set é zerado
	FD_ZERO(&rset);
	for ( ; ; ) {

    /*================================================================================

                                COMUNICAÇÃO COM O SERVIDOR

    ================================================================================*/
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
			if (fgets(sendline, MAXLINE, fp) == NULL)
				return;
      else{
        send = 1;
  			write(sockfd, sendline, strlen(sendline));
      }
		}



    /*================================================================================

                                APLICAÇÃO DA FORCA

    ================================================================================*/
    // Modo de Menu
    if (cur_state == menu){
      used_letters[0] = '\0';
      init_scr();
      game_menu();

      if (recvline[0] == '1'){
        printf("\nVocê começou uma partida simples!\n");
        cur_state = simple_game;
      }
      else if (!strcmp(recvline, "CARRASCO")){
        printf("\nCarrasco a ser implementado\n");
      }
      else if (!strcmp(recvline, "MULTIPLAYER")){
        printf("\nMultiplayer a ser implementado\n");
      }
      else if (!strcmp(recvline, "EXIT")){
        clear_scr();
        printf("Até logo!\n");
        return;
      }
    }

    // Modo de Simple Game
    if (cur_state == simple_game){

      if (!strcmp(recvline, "exit")){
        cur_state = menu;
        printf("\nVocês desistiu da partida.\n");
        printf("\nPressione qualquer tecla para voltar ao menu\n");
      }else{

        // Atualiza a vida local
        lives = recvline[1] - '0';
        if (lives == 0){
          printf("\n\nVocê perdeu! =(\n");
          printf("\nPressione qualquer tecla para voltar ao menu\n");
          //strcpy(used_letters, "\0");
          cur_state = menu;
        }
        else{
          // Verifica se é a primeira chamada
          if ((recvline[3] != '0') && (recvline[3] != 'E')){

            // Atualiza as letras utilizadas
            if (used_letters[0] == ' '){
              used_letters[0] = recvline[2];
            } else {
              update_used_letters(recvline[2], used_letters);
            }
          }

          // Atualiza a palavra local do cliente
          cli_word = malloc (strlen(recvline)-4);
          j=0;
          for (int i=4; i<strlen(recvline); i++){
            cli_word[j++]=recvline[i];
          }

          // Em caso de vitória o jogo é encerrado
          if (recvline[3] == 'S'){
            printf("\n\nParabéns, você acertou!\n");
            printf("\nA palavra era: %s\n", cli_word);
            printf("\nPressione qualquer tecla para voltar ao menu\n");
            strcpy(used_letters, " ");
            cur_state = menu;
          } else {
          // Caso contrário segue o jogo
            init_scr();
            printf("MODO DE JOGO SIMPLES\n\n");
            // Verifica acertos
            if (recvline[3] == 'T')
              printf("\nBelo chute!\n");
            else if (recvline[3] == 'F')
              printf("\nErrouu!\n");
            else if (recvline[3] == 'R')
              printf("\nA letra '%c' já foi utilizada!\n", recvline[2]);
            else if (recvline[3] == 'E')
              printf("\nCaractere inválido inserido\n");
            game(lives, cli_word, used_letters);
          }

          free(cli_word);
        }
      }
    }

	}
}
