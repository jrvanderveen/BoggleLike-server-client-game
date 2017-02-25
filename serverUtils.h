// server/game utility functions
#ifndef SERVERUTILS
#define SERVERUTILS

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
void sendInitialGameInfo(int clientSocket, unsigned char player, uint8_t boardSize, uint8_t turnLength);
bool sendRoundInfo(struct gameState game, struct player player1, struct player player2);
bool sendTurnInfo(struct gameState game, struct player player1, struct player player2);
char *recieveClientGuess(struct player, uint8_t turnLength);
bool validateGuess(char board[] ,char guess[]);
bool sendTurnResults(char guess[], struct player player1, struct player player2, bool player1Guess, bool validGuess);
void createBoard(char board[], int boardSize);
bool isVowel(char c);
char randVowel();

#endif
