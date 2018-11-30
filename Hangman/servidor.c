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
int update_used_letters (char new_letter, char *used_letters);

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

// Realiza comunicação e aplicação com o cliente
/* Comunicação através de Arrays de códigos:

    i 0 1 2 3 4 (...)
array _ _ _ _ ___
      | | | |  |---- String com a Palavra Oculta: palavra com as devidas letras
      | | | |         escondidas.
      | | | |---- Caractere que indica se o cliente acertou o chute (T: acertou,
      | | |        F: errou, S: acertou a palavra, R: caracter inválido e
      | | |          M: caracter repetido).
      | | |---- Caractere escolhido pelo usuário.
      | |---- Número de vidas do cliente.
      |---- Modo de Jogo atual.
*/
void doit(int connfd, struct sockaddr_in clientaddr) {
   char recvline[MAXDATASIZE + 1];
   char sendline[MAXDATASIZE + 1];

   int n, i;
   socklen_t remoteaddr_len = sizeof(clientaddr);

   enum state cur_state = menu;

   char *word, *hide_word;
   char used_letters[26] = " ";
   int word_len;

   char client_lives[12];
   int int_lives;

   char *hit;
   int n_hit = 0;
   char guess[2];

   while ((n = read(connfd, recvline, MAXDATASIZE)) > 0) {
      recvline[n] = 0;

      if (getpeername(connfd, (struct sockaddr *) &clientaddr, &remoteaddr_len) == -1) {
         perror("getpeername() failed");
         return;
      }

      //============================= MENU ====================================
      if (cur_state == menu){
        n_hit = 0;
        used_letters[0] = '\0';
        // Request para Simple Game
        if (recvline[0] == '1'){

          // Escolhe uma palavra "aleatória" do banco de dados
          word = get_word();

          // Inicializa a vida do cliente em 6
          client_lives[0] = '6';
          int_lives = 6;

          // Cria a "Palavra Oculta" para o cliente
          word_len = strlen(word) - 1;
          hide_word = malloc (word_len);

          for (i=0; i < word_len; i++){
              hide_word[i] = '_';
          }
          hide_word[i]=0;

          // Monta a Array para o cliente
          strcpy(sendline, "1");          // Campo de Modo de Jogo
          strcat(sendline, client_lives); // Número de vidas do cliente
          strcat(sendline, "00");         //Campo de chute e acerto ainda vazios
          strcat(sendline, hide_word);    // Palavra oculta

          cur_state = simple_game;        // Muda para o estado de Simple Game
        }
        //CARRASCO REQUEST
        else if (recvline[0] == '2'){
          strcpy(sendline, "CARRASCO");
        }
        //MULTIPLAYER REQUEST
        else if (recvline[0] == '3'){
          strcpy(sendline, "MULTIPLAYER");
        }
        // EXIT REQUEST
        else if (recvline[0] == '4'){
          strcpy(sendline, "EXIT");
          write(connfd, sendline, strlen(sendline));
          sleep(1);
          return;
        }
        else {
          strcpy(sendline, "INVALID");
        }
      }


      //========================== SIMPLE GAME =================================
      else if (cur_state == simple_game){
        //Verifica se é um pedido de saída
        if (!strcmp(recvline, "exit\n")){
          cur_state = menu;
          strcpy(sendline, "exit");

        } else {

          hit = "F";
          // Obtém o chute do cliente
          // Caso seja letra minúscula é transformado
          if ((recvline[0] >= 'a') && (recvline[0] <= 'z'))
            guess[0] = recvline[0] - 32;
          else if ((recvline[0] >= 'A') && (recvline[0] <= 'Z'))
            guess[0] = recvline[0];
          //  No último caso o caracter é inválido
          else {
            guess[0] = '0';
            hit = "E";
          }
          guess[1] = 0;

          if (hit[0] == 'F'){

            if (used_letters[0] == ' '){
              used_letters[0] = guess[0];
            }
            // Caso o caracter tenha sido utilizado retorna a flag R
            if (!update_used_letters(guess[0], used_letters))
                hit = "R";
            else {
              //Procura a letra palpite na palavra, atualizando a palavra obscura
              for (i=0; i<word_len; i++){
                if (word[i] == guess[0]){
                  n_hit++;
                  hit = "T";                  // Indica o acerto
                  hide_word[i] = guess[0];    // Atualiza a palavra obscura
                }
              }
              // Caso não tenha acertado a letra
              if (hit[0] == 'F') {
                int_lives--;                            // Atualiza a vida
                sprintf(client_lives, "%d", int_lives); // Transfere para char
                if (int_lives == 0){  // Caso chegue a zero o jogo será finalizado
                  cur_state = menu;
                }
              }
              // Verifica se todos caracteres foram acertados
              if (n_hit == word_len){
                hit = "S";
                cur_state = menu;
              }
            }
          }

          // Envia informações ao cliente
          // Tais variáveis são strings, pois strcat aceita apenas strings
          strcpy(sendline, "1");          // Modo de jogo
          strcat(sendline, client_lives); // Vida do cliente
          strcat(sendline, guess);        // Campo de chute
          strcat(sendline, hit);          // Acerto
          strcat(sendline, hide_word);    // Palavra oculta
        }
      }

      //========================== MULTIPLAYER =================================
      else if (cur_state == multiplayer){

      }
      // printf("Palavra: %s\n", word);
      printf("Linha: %s. Fim da Linha.\n", sendline);
      write(connfd, sendline, strlen(sendline));
   }
}

// Função para obter palavra aleatória do banco
char *get_word(){
  // Obtém um inteiro aleatório até 10275 (número total de palavras)
  srand(time(NULL));
  int line_number = ( rand() % 10275 );

  // Abre arquivo com palavras
  FILE *file = fopen("dicionario.txt", "r");

  // Percorre o arquivo até a linha desejada
  int count = 0;
  if ( file != NULL ){
    char line[64];
    while (fgets(line, sizeof line, file) != NULL){
        // Ao obter a linha desejada a retorna
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

// Atualiza o array de caracteres utilizados e retorna 0 caso o caracter já
//tenha sido usado
int update_used_letters (char new_letter, char *used_letters){

  char aux_letters[26];
  strcpy(aux_letters, used_letters);

  int i;
  for (i = 0; i<strlen(aux_letters); i++){
    if (aux_letters[i] == new_letter){
      return 0;
    }
    else if (aux_letters[i] > new_letter){
      used_letters[i] = new_letter;
      for (int j = i; j<strlen(aux_letters); j++){
        used_letters[j+1] = aux_letters[j];
      }

      return 1;
    }
  }
  used_letters[i+1] = used_letters[i];
  used_letters[i] = new_letter;

  return 1;
}
