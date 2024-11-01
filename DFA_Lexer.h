// this is the header for my lexer so I can call the functions from other files

#ifndef DFA_LEXER_H
#define DFA_LEXER_H

#include <stdio.h> // standard i/o lib for C
#include <stdlib.h> // standard lib for C
#include <ctype.h> // lib for char type functions like isdigit()
#include <string.h> // lib for string functions like strcmp()

#define MAX_LEXEME_LEN 1024 // maximum length for lexeme
#define TOKEN_STREAM_LEN 10 // initial stream length

//type definition using enum to valuate type in a readable way
typedef enum {
        IF, ELSE, VOID, CASE, DEFAULT, BREAK, CONTINUE, WHILE, FOR, DO, RETURN, CONST, LET, VAR, DEFINE, CLASS, OBJECT, THIS, SUPER, EXTENDS, IMPLEMENTS, INTERFACE, TRY, CATCH, FINALLY, THROW, PUBLIC, PRIVATE, PROTECTED, STATIC, FINAL, IMPORT, EXPORT, ASYNC, AWAIT,
        IDENTIFIER,
        // operators 1st order
        PLUS, MINUS, TIMES, DIVIDE, MOD, LESS, GREATER, BOR, BAND, BXOR, NEGATION, EQUAL, 
        // second order        
        INCREMENT, DECREMENT, POWER, SLCOMMENT, DOUBLEMOD, BSLEFT, BSRIGHT, OR, AND, EXP, DOUBLENEGATION, EQUALITY, 
        // third order
        PLUSEQUALS, MINUSEQUALS, TIMESEQUALS, DIVIDEEQUALS, MODEQUALS, LESSEQUALS, GREATEREQUALS, OREQUALS, ANDEQUALS, XOREQUALS, NOTEQUALS,
        SEMI, COMMA, OPENP, CLOSEP, OPENC, CLOSEC, OPENB, CLOSEB,
        INT, DOUBLE, STRING,
        UNKNOWN, INVALID,
} Type;
// token structure consisting of a Type and string for the actual value of the token
typedef struct {
        Type type;
        char lexeme[MAX_LEXEME_LEN];
        size_t line;
        size_t col;
} Token;

typedef struct TrieNode {
        char *value; // Value of the node (substring)
        int isEnd; // Indicates if this node marks the end of a word
        struct TrieNode **children; // Array of child nodes
        int childCount; // Number of children
        Type type; // Type associated with the node
} TrieNode;

// simple initialization of trie node setting it to empty values and creating new memory space
TrieNode *createNode(const char *word, Type type);
// Function to free children of the trie
void freeTrie(TrieNode *node);
// Function to find the longest common prefix
int prefix(const char *value1, const char *value2);
void insert(TrieNode **root, const char *word, Type type);
Type search(TrieNode *root, const char *word);
Token operators(char *current);
// this should be pretty self explanitory
Token seperator(char *current);
// Identifier state machine see notes in DFA Images branch of this repository to see how I got this or just sift through the logic its not too complicated
Token identifier(char *current);
// Number DFA Logic same thing as the Identifier state machine see DFA Images branch for notes or just logic your way through
Token number(char *current);
// string literal DFA once again check DFA Images for notes on how I came up with this it would also help to read the lexical analysis test file for more info or just logic through it
Token string(char *current);
// Main lexing method
Token *lexer_main(char *input);

#endif