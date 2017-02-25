#include <unistd.h>
#include <poll.h>
#include <stdio.h>
#include <stdbool.h>

void helper(char test[]){
  int i;
  char holder[10];
  for(i = 0; i < 10; i++){
    holder[i] = test[i];
  }
  for(i = 0; i < 9; i++){
    if(i%2 == 0){
      holder[i]= NULL;
    }
    printf("%c ", holder[i]);
  }
}

int main()
{
  char test[10] = {"testing"};
  helper(test);
  printf("%s\n",test);
    return 0;
}
