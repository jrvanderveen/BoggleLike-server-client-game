// C implementation of search, insert, fill, and free operations
// on Trie
#ifndef TRIE
#define TRIE
#define ALPHABET_SIZE (26)


struct TrieNode
{
    struct TrieNode *children[ALPHABET_SIZE];
    bool isLeaf;
};
struct TrieNode *getNode(void);
void fillTree(struct TrieNode *root, const char *textFile);
void insert(struct TrieNode *root, const char *key);
bool search(struct TrieNode *root, const char *key);
void freeTree(struct TrieNode *root);
void freeChildren(struct TrieNode *node);

#endif
