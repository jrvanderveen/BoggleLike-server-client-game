/* client.c - code for example client program that uses TCP */

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <unistd.h>
#include <inttypes.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <stdbool.h>
#include <poll.h>
#include <errno.h>

struct gameState{
  uint8_t boardSize;
  uint8_t turnLength;
  uint8_t whoAmI;
};
uint8_t *recieveInitialGameInfo(int sd);
bool recieveRoundInfo(int sd, uint8_t boardSize, uint8_t activePlayer);
bool recieveTurnInfo(int sd);
void sendGuess(int sd, char guess[]);
bool recieveGuessResults(int sd, bool turn);
// #include "clientUtils.h"
/*------------------------------------------------------------------------
* Program: client
*
* Purpose: allocate a socket, connect to a server, and print all output
*
* Syntax: client [ host [port] ]
*
* host - name of a computer on which server is executing
* port - protocol port number server is using
*
* Note: Both arguments are optional. If no host name is specified,
* the client uses "localhost"; if no protocol port is
* specified, the client uses the default given by PROTOPORT.
*
*------------------------------------------------------------------------
*/
int main( int argc, char **argv) {
	struct hostent *ptrh; /* pointer to a host table entry */
	struct protoent *ptrp; /* pointer to a protocol table entry */
	struct sockaddr_in sad; /* structure to hold an IP address */
	int sd; /* socket descriptor */
	int port; /* protocol port number */
	char *host; /* pointer to host name */
	int n; /* number of characters read */
	char SRbuff[1000];
	uint8_t boardSize;
	bool gameActive = true;
	bool turnActive = true;

//////////////////////////////////////////////////////////////////////////////////////////////
//set up client
	memset((char *)&sad,0,sizeof(sad)); /* clear sockaddr structure */
	sad.sin_family = AF_INET; /* set family to Internet */

	if( argc != 3 ) {
		fprintf(stderr,"Error: Wrong number of arguments\n");
		fprintf(stderr,"usage:\n");
		fprintf(stderr,"./client server_address server_port\n");
		exit(EXIT_FAILURE);
	}

	port = atoi(argv[2]); /* convert to binary */
	if (port > 0) /* test for legal value */
		sad.sin_port = htons((u_short)port);
	else {
		fprintf(stderr,"Error: bad port number %s\n",argv[2]);
		exit(EXIT_FAILURE);
	}

	host = argv[1]; /* if host argument specified */

	/* Convert host name to equivalent IP address and copy to sad. */
	ptrh = gethostbyname(host);
	if ( ptrh == NULL ) {
		fprintf(stderr,"Error: Invalid host: %s\n", host);
		exit(EXIT_FAILURE);
	}

	memcpy(&sad.sin_addr, ptrh->h_addr, ptrh->h_length);

	/* Map TCP transport protocol name to protocol number. */
	if ( ((long int)(ptrp = getprotobyname("tcp"))) == 0) {
		fprintf(stderr, "Error: Cannot map \"tcp\" to protocol number");
		exit(EXIT_FAILURE);
	}

	/* Create a socket. */
	sd = socket(PF_INET, SOCK_STREAM, ptrp->p_proto);
	if (sd < 0) {
		fprintf(stderr, "Error: Socket creation failed\n");
		exit(EXIT_FAILURE);
	}

	/* Connect the socket to the specified server. */
	if (connect(sd, (struct sockaddr *)&sad, sizeof(sad)) < 0) {
		fprintf(stderr,"connect failed\n");
		exit(EXIT_FAILURE);
	}
//end setup
//////////////////////////////////////////////////////////////////////////////////////////////
    struct pollfd mypoll = { STDIN_FILENO, POLLIN|POLLPRI };
		bool turn;
		char guess[255];
		uint8_t *buf;
		struct gameState game;
		buf = recieveInitialGameInfo(sd);
		game.boardSize = buf[0];
		game.turnLength = buf[1];
		game.whoAmI = buf[2];
		//game loop
		while (gameActive) {
			gameActive = recieveRoundInfo(sd, game.boardSize, game.whoAmI);
			if(gameActive){
				turnActive = true;
				while(turnActive){
					turn = recieveTurnInfo(sd);
					if(turn == false){
						fprintf(stderr,"Please wait for opponent to enter word... \n");
					}
					else{
						fprintf(stderr,"Your turn, %d (sec) to enter word: ", game.turnLength);
						if(poll(&mypoll,1,game.turnLength*1000)){
							scanf("%254s",guess);
							guess[strlen(guess)] = '\0';
							sendGuess(sd,guess);
						}
						else{
							fprintf(stderr, "Failed to send word in time limit.\n");
							guess[0] = '0';
							guess[1] = '\0';
						}

					}
					turnActive = recieveGuessResults(sd,turn);
				}
			}
		}
	close(sd);

}

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
