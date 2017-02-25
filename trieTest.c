#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>

#include "trie.h"

#define ARRAY_SIZE(a) sizeof(a)/sizeof(a[0])

void fillTrees(struct TrieNode *root, const char *textFile){
  char keys[][8] = {"the", "a", "there", "answer", "any",
                   "by", "bye", "their"};
  int i;
  for (i = 0; i < ARRAY_SIZE(keys); i++){
      insert(root, keys[i]);
  }
}

int main()
{
    // Input keys (use only 'a' through 'z' and lower case)
    char keys[][8] = {"the", "a", "there", "answer", "any",
                     "by", "bye", "their"};

    char output[][32] = {"Not present in trie", "Present in trie"};


    struct TrieNode *root = getNode();

    // // Construct trie
    // int i;
    // for (i = 0; i < ARRAY_SIZE(keys); i++){
    //     insert(root, keys[i]);
    // }
    fillTrees(root, "someString");
    // Search for different keys
    printf("%s --- %s\n", "the", output[search(root, "the")] );
    printf("%s --- %s\n", "these", output[search(root, "these")] );
    printf("%s --- %s\n", "their", output[search(root, "their")] );
    printf("%s --- %s\n", "thaw", output[search(root, "thaw")] );

    freeTree(root);
    // free(root);

    printf("test after free\n");
    printf("%s --- %s\n", "the", output[search(root, "the")] );
    printf("%s --- %s\n", "these", output[search(root, "these")] );
    printf("%s --- %s\n", "their", output[search(root, "their")] );
    printf("%s --- %s\n", "thaw", output[search(root, "thaw")] );

    return 0;
}
