//////////////////////////////////////////////////////

// Code credit goes to http://www.geeksforgeeks.org
//////////////////////////////////////////////////////

// C implementation of search and insert operations
// on Trie
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include "trie.h"

#define ARRAY_SIZE(a) sizeof(a)/sizeof(a[0])

// Alphabet size (# of symbols)
#define ALPHABET_SIZE (26)

// Converts key current character into index
// use only 'a' through 'z' and lower case
#define CHAR_TO_INDEX(c) ((int)c - (int)'a')
#define INDEX_TO_CHAR(i) ((int)c + (int)'a')

// trie node
// struct TrieNode
// {
//     struct TrieNode *children[ALPHABET_SIZE];
//
//     // isLeaf is true if the node represents
//     // end of a word
//     bool isLeaf;
// };

// Returns new trie node (initializefree_alld to NULLs)
struct TrieNode *getNode(void)
{
    struct TrieNode *pNode = NULL;

    pNode = (struct TrieNode *)malloc(sizeof(struct TrieNode));

    if (pNode)
    {
        int i;

        pNode->isLeaf = false;

        for (i = 0; i < ALPHABET_SIZE; i++)
            pNode->children[i] = NULL;
    }

    return pNode;
}
/////////
void fillTree(struct TrieNode *root, const char *textFile){
  if(strlen(textFile) == 0){
    fprintf(stderr,"Error: .txt file \n");
		exit(EXIT_FAILURE);
  }
  FILE* file = fopen(textFile, "r"); /* should check the result */
  char line[256];
  int wordLen;
  while (fgets(line, sizeof(line), file)) {
      /* note that fgets don't strip the terminating \n, checking its
         presence would allow to handle lines longer that sizeof(line) */
    wordLen = strlen(line);
    if(wordLen > 0){
      if (line[wordLen - 1] == '\n') {
        line[wordLen - 1] = '\0';
      }
      insert(root, line);
    }
  }
  fclose(file);
}

// If not present, inserts key into trie
// If the key is prefix of trie node, just marks leaf node
void insert(struct TrieNode *root, const char *key)
{
    int level;
    int length = strlen(key);
    int index;
    struct TrieNode *pCrawl = root;

    for (level = 0; level < length; level++)
    {
        index = CHAR_TO_INDEX(key[level]);
        if (!pCrawl->children[index])
            pCrawl->children[index] = getNode();

        pCrawl = pCrawl->children[index];
    }

    // mark last node as leaf
    pCrawl->isLeaf = true;
}

// Returns true if key presents in trie, else false
bool search(struct TrieNode *root, const char *key)
{
    int level;
    int length = strlen(key);
    int index;
    struct TrieNode *pCrawl = root;

    for (level = 0; level < length; level++)
    {
        index = CHAR_TO_INDEX(key[level]);

        if (!pCrawl->children[index])
            return false;

        pCrawl = pCrawl->children[index];
    }

    return (pCrawl != NULL && pCrawl->isLeaf);
}



void freeChildren(struct TrieNode *node){
    int i;
    if (node != NULL && node->isLeaf == false){
      for (i=0;i<ALPHABET_SIZE;i++){
        if (node->children[i] != NULL){
            freeChildren(node->children[i]);
            node->children[i] = NULL;
        }
      }
    }

    if (node != NULL){
      free(node);
   }
}

void freeTree(struct TrieNode *root){
  freeChildren(root);
  root = NULL;
}





//
