
#include <sys/socket.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <stdint.h>
#include <stdbool.h>
#include <string.h>
#include <errno.h>

#include "clientUtils.h"

uint8_t *recieveInitialGameInfo(int sd){
  static uint8_t gameInfo[3];
  char SRbuff;
  recv(sd, &SRbuff, sizeof(SRbuff), 0);
  fprintf(stderr,"\nYou are player: %c, ",SRbuff);
  recv(sd, &gameInfo[0], sizeof(gameInfo[0]), 0);
  fprintf(stderr,"Board size: %d, ",gameInfo[0]);
  recv(sd, &gameInfo[1], sizeof(gameInfo[1]), 0);
  fprintf(stderr,"Turn length(sec): %d\n",gameInfo[1]);
  if(SRbuff == '1'){
    fprintf(stderr,"Wating for player 2...");
    gameInfo[2] = 1;
  }
  else{
    gameInfo[2] = 2;
  }
  return gameInfo;
}

bool recieveRoundInfo(int sd, uint8_t boardSize, uint8_t activePlayer){
  uint8_t player1score;
  uint8_t player2score;
  char board[boardSize];
  uint8_t roundNum;
  int n, i;
  if(recv(sd, &player1score, sizeof(player1score), MSG_PEEK | MSG_DONTWAIT) == 0){
    close(sd);
    exit(EXIT_FAILURE);
  }
  recv(sd, &player1score, sizeof(player1score), 0);
  recv(sd, &player2score, sizeof(player2score), 0);

  if(player1score >= 3){
    if(activePlayer == 1){
      fprintf(stderr, "You Won!\n");
    }
    else{
      fprintf(stderr, "You lost!\n");
    }
    return false;
  }
  if(player2score >= 3){
    if(activePlayer == 1){
      fprintf(stderr, "You lost!\n");
    }
    else{
      fprintf(stderr, "You Won!\n");
    }
    return false;
  }
  else{
    recv(sd, &roundNum, sizeof(roundNum), 0);
    fprintf(stderr, "\n\nRound %d...\n", roundNum);
    fprintf(stderr, "Score: %d - %d\n", player1score, player2score);
    n = recv(sd, board, boardSize, 0);
    board[n] = '\0';
    fprintf(stderr, "board: ");
    for(i = 0; i < boardSize; i++){
      fprintf(stderr, "%c ", board[i]);
    }
    fprintf(stderr, "\n");
    return true;
  }
}

bool recieveTurnInfo(int sd){
  char turn;

  if(recv(sd, &turn, sizeof(turn), MSG_PEEK | MSG_DONTWAIT) == 0){
    close(sd);
    exit(EXIT_FAILURE);
  }
  recv(sd, &turn, sizeof(turn),0);
  if(turn == 'Y'){
    return true;
  }
  return false;
}

void sendGuess(int sd, char guess[]){
  uint8_t len = strlen(guess);
  int n;
  n = send(sd,&len,sizeof(len),MSG_NOSIGNAL);
  if(n == -1){
    close(sd);
    exit(EXIT_FAILURE);
  }
  n = send(sd,guess,len,MSG_NOSIGNAL);
  if(n == -1){
    close(sd);
    exit(EXIT_FAILURE);
  }
}

bool recieveGuessResults(int sd, bool turn){
  uint8_t result;
  int n;
  char guess[255];
  if(recv(sd, &result, sizeof(result), MSG_PEEK | MSG_DONTWAIT) == 0){
    close(sd);
    fprintf(stderr, "server closed\n");
    exit(EXIT_FAILURE);
  }

  recv(sd, &result, sizeof(result), 0);

  if(turn){
    if(result == 0){
      fprintf(stderr, "Invalid word!\n");
      return false;
    }
    else{
      fprintf(stderr, "Valid word!\n");
      return true;
    }
  }
  else{
    if(result == 0){
      fprintf(stderr, "Opponent lost the round!\n");
      return false;
    }
    else{
      recv(sd, guess, result, 0);
      guess[result] = '\0';
      fprintf(stderr, "Opponent entered: %s\n", guess);
      return true;
    }
  }
}



















//
