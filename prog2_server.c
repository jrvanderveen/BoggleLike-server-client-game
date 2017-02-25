/* server.c - code for example server program that uses TCP */

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <stdbool.h>
#include <time.h>
#include <stdint.h>
#include <errno.h>

#include "trie.h"
// #include "serverUtils.h"

#define QLEN 6 /* size of request queue */
int visits = 0; /* counts client connections */

struct gameState{
  uint8_t roundNum;
  int maxRoundsWon;
  bool player1Turn;
  bool roundActive;
  struct TrieNode *roundGuesses;
  char *board;
};
struct player{
  int playerNum;
  uint8_t roundsWon;
  int uniqueWords;
  int connectionSocket;
  bool connected;
};
const char vowels[] = {'a','e','i','o','u'};
void sendInitialGameInfo(int clientSocket, unsigned char player, uint8_t boardSize, uint8_t turnLength);
bool sendRoundInfo(struct gameState game, struct player player1, struct player player2);
bool sendTurnInfo(struct gameState game, struct player player1, struct player player2);
char *recieveClientGuess(struct player, uint8_t turnLength);
bool validateGuess(char board[] ,char guess[]);
bool sendTurnResults(char guess[], struct player player1, struct player player2, bool player1Guess, bool validGuess);
void createBoard(char board[], int boardSize);
bool isVowel(char c);
char randVowel();


/*------------------------------------------------------------------------
* Program: server
*newClie
* Purpose: allocate a socket and then repeatedly execute the following:
* (1) wait for the next connection from a client
* (2) send a short message to the client
* (3) close the connection
* (4) go back to step (1)
*
* Syntax: server [ port ]
*
* port - protocol port number to use
*
* Note: The port argument is optional. If no port is specified,
* the server uses the default given by PROTOPORT.
*
*------------------------------------------------------------------------
*/

int main(int argc, char **argv) {
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold server's address */
	struct sockaddr_in cad; /* structure to hold client's address */
	int sd, clientSocket[2]; /* socket descriptors */
	int n;
	int port; /* protocol port number */
	int alen; /* length of address */
	int optval = 1; /* boolean value when we set socket option */
	struct TrieNode *dicRoot = getNode();
	bool validG;

	signal(SIGCHLD,SIG_IGN);

	if( argc != 5 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./server server_port board_size turn_length_sec dictionary\n");
		exit(EXIT_FAILURE);
	}

	uint8_t turnLength = atoi(argv[3]);
	uint8_t boardSize = atoi(argv[2]);
	char board[boardSize+1];
//////////////////////////////////////////////////////////////////////////////////////////////
//setting up server
	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */
	sad.sin_addr.s_addr = INADDR_ANY; /* set the local IP address */

	port = atoi(argv[1]); /* convert argument to binary */
	if (port > 0) { /* test for illegal value */
		sad.sin_port = htons((u_short)port);
	} else { /* print error message and exit */
		fprintf(stderr,"Error: Bad port number %s\n",argv[1]);
		exit(EXIT_FAILURE);
	}

	/* Map TCP transport protocol name to protocol number */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	if( setsockopt(sd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval)) < 0 ) {
		fprintf(stderr, "Error Setting socket option failed\n");
		exit(EXIT_FAILURE);
	}

	/* Bind a local address to the socket */
	if (bind(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"Error: Bind failed\n");
		exit(EXIT_FAILURE);
	}

	/* Specify size of request queue */
	if (listen(sd, QLEN) < 0) {
		fprintf(stderr,"Error: Listen failed\n");
		exit(EXIT_FAILURE);
	}
//end set up
//////////////////////////////////////////////////////////////////////////////////////////////

	fillTree(dicRoot, argv[4]);

	int numClients = 0;
	bool findClients = true;
	bool clientsConnected = true;
	/* Main server loop - accept and handle requests */
	while (findClients) {
		close(clientSocket[0]);
		close(clientSocket[1]);
		while(numClients < 2){
			alen = sizeof(cad);
			if ((clientSocket[numClients]=accept(sd, (struct sockaddr *)&cad, &alen)) < 0) {
				fprintf(stderr, "Error: Accept failed: %d\n",errno);
				exit(EXIT_FAILURE);
			}
			else{
				sendInitialGameInfo(clientSocket[numClients], ((unsigned char) (numClients + 49)), (unsigned char) boardSize, (unsigned char) turnLength);
				numClients++;
			}
		}
		if(numClients == 2){
			numClients = 0;
			pid_t pid = fork();
			if(pid < 0 ){
				perror("fork");
				return -1;
			}
			else{
				 if(pid == 0){
					 close(sd);
					 findClients = false;
				 }
			}
		}
	}

	struct player player1;
	player1.playerNum = 1;
	player1.roundsWon = 0;
	player1.uniqueWords = 0;
	player1.connectionSocket = clientSocket[0];
	player1.connected = true;

	struct player player2;
	player2.playerNum = 2;
	player2.roundsWon = 0;
	player2.uniqueWords = 0;
	player2.connectionSocket = clientSocket[1];
	player2.connected = true;

	struct gameState game;
	game.player1Turn = true;
	game.roundActive = true;
	game.roundNum = 1;
	game.maxRoundsWon = 0;
//game loop
	char stall[100];
	char *guess;

	while(game.maxRoundsWon < 3){
		createBoard(board,boardSize);
		game.board = board;
		game.roundGuesses = getNode();
		clientsConnected = sendRoundInfo(game, player1, player2);
		if(clientsConnected == false){
			close(player1.connectionSocket);
			close(player2.connectionSocket);
			exit(EXIT_FAILURE);
		}
		while(game.roundActive){
			validG = false;
			clientsConnected = sendTurnInfo(game, player1, player2);
			if(clientsConnected == false){
				close(player1.connectionSocket);
				close(player2.connectionSocket);
				exit(EXIT_FAILURE);
			}
			if(game.player1Turn){
				guess = recieveClientGuess(player1, turnLength);
			}
			else{
				guess = recieveClientGuess(player2, turnLength);
			}
			if(strlen(guess) == 0){
				close(player1.connectionSocket);
				close(player2.connectionSocket);
				exit(EXIT_FAILURE);
			}
			if(validateGuess(game.board ,guess)){
				if(search(dicRoot, guess) && !search(game.roundGuesses, guess)){
					insert(game.roundGuesses, guess);
					validG = true;
				}
			}

			game.roundActive = validG;
			clientsConnected = sendTurnResults(guess, player1, player2, game.player1Turn, validG);
			if(clientsConnected == 0){
				close(player1.connectionSocket);
				close(player2.connectionSocket);
				exit(EXIT_FAILURE);
			}
			game.player1Turn = !game.player1Turn;
		}
		freeTree(game.roundGuesses);
		player1.roundsWon += game.player1Turn;
		player2.roundsWon += !game.player1Turn;
		game.roundNum++;
		game.player1Turn = game.roundNum % 2;
		if(player1.roundsWon < 3 && player2.roundsWon < 3){
			game.roundActive = true;
		}
		else{
			game.maxRoundsWon = 3;
		}
	}
		sendRoundInfo(game, player1, player2);
		close(clientSocket[0]);
		close(clientSocket[1]);
}

////////////////////////////////////////////////////////////////////////////////////////////////
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
