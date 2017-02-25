// server/game utility functions
#ifndef CLIENTUTILS
#define CLIENTUTILS

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
#endif
