#include <sys/socket.h>
#include <sys/types.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <time.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <errno.h>

#include "serverUtils.h"
#include "trie.h"

const char vowels[] = {'a','e','i','o','u'};
//////////////////game structure
// struct gameState{
//   uint8_t roundNum;
//   int maxRoundsWon;
//   bool player1Turn;
//   bool roundActive;
//   struct TrieNode *roundGuesses;
//   char *board;
// };
// struct player{
//   int playerNum;
//   uint8_t roundsWon;
//   int uniqueWords;
//   int connectionSocket;
// };

//////////////////sending client game information
//probably change from void to int for error handleing
void sendInitialGameInfo(int clientSocket, unsigned char player, uint8_t boardSize, uint8_t turnLength){
  int n;
  n = send(clientSocket,&player,sizeof(player), MSG_NOSIGNAL);
  n = send(clientSocket,&boardSize,sizeof(boardSize),0);
  n = send(clientSocket,&turnLength,sizeof(turnLength),0);
}

bool sendRoundInfo(struct gameState game, struct player player1, struct player player2){
  int n;
  if(player1.connected){
    n = send(player1.connectionSocket,&player1.roundsWon,sizeof(player1.roundsWon),MSG_NOSIGNAL);
    if(n == -1){
      return false;
    }
    n = send(player1.connectionSocket,&player2.roundsWon,sizeof(player2.roundsWon),MSG_NOSIGNAL);
    if(n == -1){
      return false;
    }
  }
  if(player2.connected){
    n = send(player2.connectionSocket,&player1.roundsWon,sizeof(player1.roundsWon),MSG_NOSIGNAL);
    if(n == -1){
      return false;
    }
    n = send(player2.connectionSocket,&player2.roundsWon,sizeof(player2.roundsWon),MSG_NOSIGNAL);
    if(n == -1){
      return false;
    }
  }
  if(player1.roundsWon < 3 && player1.roundsWon < 3 && player1.connected & player2.connected){
    n = send(player1.connectionSocket,&game.roundNum,sizeof(game.roundNum),MSG_NOSIGNAL);
    if(n == -1){
      return false;
    }
    n = send(player2.connectionSocket,&game.roundNum,sizeof(game.roundNum),MSG_NOSIGNAL);
    if(n == -1){
      return false;
    }
    n = send(player1.connectionSocket,game.board,strlen(game.board),MSG_NOSIGNAL);
    if(n == -1){
      return false;
    }
    n = send(player2.connectionSocket,game.board,strlen(game.board),MSG_NOSIGNAL);
    if(n == -1){
      return false;
    }
  }
  return true;
}

bool sendTurnInfo(struct gameState game, struct player player1, struct player player2){
  char turnOutPut[2] = {'N','Y'};
  int n;
  n = send(player1.connectionSocket,&turnOutPut[game.player1Turn],sizeof(char), MSG_NOSIGNAL);
  if(n == -1){
    return false;
  }
  n = send(player2.connectionSocket,&turnOutPut[!game.player1Turn],sizeof(char), MSG_NOSIGNAL);
  if(n == -1){
    return false;
  }
  return true;
}
/////////////////////////////////////////////////////////////////////////
char *recieveClientGuess(struct player playerx, uint8_t turnLength){
  static char guess[255];
  int n;
  uint8_t i;

  struct timeval tv;
  tv.tv_sec = turnLength;  // numSeconds is the turn length
  tv.tv_usec = 0;

  if(recv(playerx.connectionSocket, &i, sizeof(i), MSG_PEEK | MSG_DONTWAIT) == 0){
    i = 0;
  }
  else{
    setsockopt(playerx.connectionSocket,SOL_SOCKET,SO_RCVTIMEO, &tv, sizeof(struct timeval));
    n = recv(playerx.connectionSocket, &i, sizeof(i), 0);
    if( n == -1 && errno == EAGAIN ) {
      fprintf(stderr,"client timeout\n");
      guess[0] = '0';
      i = 1;
    }
    else{
      n = recv(playerx.connectionSocket, guess, i, 0);
      if( n == -1 && errno == EAGAIN ) {
        fprintf(stderr,"client timeout\n");
        guess[0] = '0';
        i = 1;
      }
    }
  }
  guess[i] = '\0';
  return guess;
}

bool validateGuess(char board[] ,char guess[]){
  int i,x,b,g, validLetters;
  validLetters = 0;
  b = strlen(board);
  g = strlen(guess);
  char holder[b];
  for(i = 0; i < b; i++){
    holder[i] = board[i];
  }
  if(b >= g){
    for(i = 0; i < g; i++){
      for(x = 0; x < b; x++){
        if(holder[x] == guess[i]){
          holder[x] = NULL;
          validLetters++;
          x = b;
        }
      }
    }
    if(validLetters == g){
      return true;
    }
  }
  return false;
}

bool sendTurnResults(char guess[], struct player player1, struct player player2, bool player1Guess, bool validGuess){
  uint8_t len = strlen(guess);
  uint8_t valid = validGuess;
  int n;
  if(validGuess){
    if(player1Guess){
      send(player1.connectionSocket,&valid,sizeof(valid), MSG_NOSIGNAL);
      if(n == -1){
        return false;
      }
      send(player2.connectionSocket,&len,sizeof(len), MSG_NOSIGNAL);
      if(n == -1){
        return false;
      }
      send(player2.connectionSocket,guess,strlen(guess), MSG_NOSIGNAL);
      if(n == -1){
        return false;
      }
    }
    else{
      send(player2.connectionSocket,&valid,sizeof(valid), MSG_NOSIGNAL);
      if(n == -1){
        return false;
      }
      send(player1.connectionSocket,&len,sizeof(len), MSG_NOSIGNAL);
      if(n == -1){
        return false;
      }
      send(player1.connectionSocket,guess,strlen(guess), MSG_NOSIGNAL);
      if(n == -1){
        return false;
      }
    }
    return true;
  }
  else{
    send(player1.connectionSocket,&valid,sizeof(valid), MSG_NOSIGNAL);
    send(player2.connectionSocket,&valid,sizeof(valid), MSG_NOSIGNAL);
  }
}
//////////////////board creation
void createBoard(char board[], int boardSize){
  int i, n;
  bool hasVowel = false;
  time_t t;
  srand((unsigned) time(&t));
  //((char) (numClients + 49))
  for(i = 0; i < boardSize; i++){
    board[i] = ((char) ((rand() % 26) + 97));
    if(isVowel(board[i])){
      hasVowel = true;
    }
  }
  if(hasVowel == false){
    board[boardSize-1] = randVowel();
  }
  board[boardSize] = '\0';
}

bool isVowel(char c){
  if(c == 'a' || c == 'e' || c == 'i' || c == 'o' || c == 'u'){
    return true;
  }
  return false;
}

char randVowel(){
  time_t t;
  srand((unsigned) time(&t));
  return vowels[rand() % 4];
}
////////////////////////////////////
