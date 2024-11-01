// here is my implementation of a lexer using DFA

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

char *strdup(const char *c)
{
        char *dup = malloc(strlen(c) + 1);

        if (dup != NULL)
           strcpy(dup, c);

        return dup;
}

typedef struct TrieNode {
        char *value; // Value of the node (substring)
        int isEnd; // Indicates if this node marks the end of a word
        struct TrieNode **children; // Array of child nodes
        int childCount; // Number of children
        Type type; // Type associated with the node
} TrieNode;

// simple initialization of trie node setting it to empty values and creating new memory space
TrieNode *createNode(const char *word, Type type) {
        TrieNode *node = malloc(sizeof(TrieNode));
        if (!node) {
                fprintf(stderr, "LEXICAL ERROR: Unable to allocate trie memory\n");
                return NULL;
        }
        node->value = strdup(word);
        node->children = NULL;
        node->isEnd = 0;
        node->childCount = 0;
        node->type = type;

        return node;
}

// Function to free children of the trie
void freeTrie(TrieNode *node) {
        if (node) {
                for (int i = 0; i < node->childCount; i++) {
                        freeTrie(node->children[i]);
                }
                free(node->value);
                free(node->children);
                free(node);
        }
}

// Function to find the longest common prefix
int prefix(const char *value1, const char *value2) {
        // simple string comparision function that returns the length of the matched values from the start / the common prefix
        // this way all comparisons of length in insert and search etc... are certain that those lengths are in fact equal in value not just length
        int i = 0;
        while (value1[i] && value2[i] && value1[i] == value2[i]) {
                i++;
        }
        return i;
}
void insert(TrieNode **root, const char *word, Type type) {
        if (!*root) { // null check for root
                printf("NULL ROOT\n");
                *root = createNode(word, type);
                return;
        }

        TrieNode *current = *root;
        while (current) {
                int len = prefix(current->value, word);
                // checking if the common prefix matches the current value aka does this prefix already exist in the current node if so look for child node
                // if an existing child already exists for a portion of that word make that current continue until no match and create a new node for the portion of the word
                // not already existing in this branch
                if (len == strlen(current->value)) {
                        // Current node matches prefix
                        if (len == strlen(word)) {
                                current->isEnd = 1; // Word already exists
                                return;
                        }
                        // set found child to 0 and look for child in below loop if common prefix is not found create new
                        int foundChild = 0;
                        for (int i = 0; i < current->childCount; i++) {
                                if (current->children[i]->value[0] == word[len]) { // looping through current children looking for match
                                        word += len; // setting word to the values we havent already used in prev nodes
                                        current = current->children[i]; // moving into the node we just matched with for next comp
                                        foundChild = 1; // skip below if statement so if there is another shared prefix existing we can move into that before creating the end node and returning
                                        break;
                                }
                        }
                        // creating new child because it does not already exist
                        if (!foundChild) {
                                TrieNode *newChild = createNode(word + len, type); // creating a new node with value of unmatched word portion word + len
                                // say hello is the word and our common prefix is len 2 the prev node would be hel the new node we create would be lo
                                newChild->isEnd = 1;
                                current->children = realloc(current->children, sizeof(TrieNode*) * (current->childCount + 1));
                                if (current->children) {
                                        current->children[current->childCount++] = newChild;
                                } else {
                                        printf("Memory allocation failed for new child\n");
                                }
                                return;
                        }
                } else { // if a common prefix is found between two existing nodes make a new node of that prefix and two new branches one for the existing branch and one for the new one
                        // Split the current node
                        TrieNode *newNode = createNode(current->value + len, current->type); // creating new node for the existing branch starting from beyond the common prefix
                        newNode->isEnd = current->isEnd;
                        newNode->children = current->children;
                        newNode->childCount = current->childCount;

                        // Update current node
                        current->value[len] = '\0'; // setting current to the common prefix vlaue
                        current->isEnd = 0; // no longer an end node since it has two more leaves branching off it
                        current->children = NULL; // resetting children since the new node is the branch previously in current
                        current->childCount = 0; // resetting child count for same reason

                        // Adding the previously existing branch as a child of the common prefix
                        current->children = realloc(current->children, sizeof(TrieNode*) * (current->childCount + 1));
                        if (current->children) {
                                current->children[current->childCount++] = newNode; // memory handling just in case memory allocation fails so we know what went wrong
                        } else {
                                printf("Memory allocation failed during split\n"); // error message in case of failed allocation
                        }

                        // now handling the new word by checking if there is any remainder or if the common prefix is the end of the word such as append as existing and appending as the new insertion word
                        // we would need a new node for the ing
                        // this will pretty much always be called unless inserting a word that already exists
                        if (strlen(word) > len) {
                                TrieNode *newChild = createNode(word + len, type); // creating a new node for the ending that does not exist
                                newChild->isEnd = 1; // setting it to an end node so we know that the word is a match if we make it here
                                current->children = realloc(current->children, sizeof(TrieNode*) * (current->childCount + 1)); // adding memory for the new child node
                                if (current->children) {
                                        current->children[current->childCount++] = newChild;
                                } else {
                                        printf("Memory allocation failed for new child\n");
                                }
                        }
                        // returning
                        return;
                }
        }
}
Type search(TrieNode *root, const char *word) {
        TrieNode *current = root;

        while (current) {
                int len = prefix(current->value, word); // Get the length of the common prefix

                // if the two words match
                if (len == strlen(current->value)) {

                        // Check if we've reached the end of the word
                        if (len == strlen(word)) {
                                return current->type; // Found the exact match
                        }

                        // Move to the next child node
                        int foundChild = 0;
                        for (int i = 0; i < current->childCount; i++) {
                                if (current->children[i]->value[0] == word[len]) {
                                        word += len;
                                        current = current->children[i]; // Move to the matching child
                                        foundChild = 1;
                                        break;
                                }
                        }

                        // If no matching child is found, return UNKNOWN
                        if (!foundChild) {
                                return IDENTIFIER; // Not found
                        }
                } else {
                        return IDENTIFIER; // Not found
                }
        }
        return IDENTIFIER; // Not found
}

Token operator(char *current) {
        Token token = {UNKNOWN, "", 0, 0};
        int len = 1;

        // def type of corresponding comp op so long as the order of the operators doesnt change it should work
        const char c = *current;
        // just a switch statement for finding the value of the first character
        switch (c) {
                case '+': token.type = PLUS; break;
                case '-': token.type = MINUS; break;
                case '*': token.type = TIMES; break;
                case '/': token.type = DIVIDE; break;
                case '%': token.type = MOD; break;
                case '=': token.type = EQUAL; break;
                case '!': token.type = NEGATION; break;
                case '<': token.type = LESS; break;
                case '>': token.type = GREATER; break;
                case '|': token.type = BOR; break;
                case '&': token.type = BAND; break;
                case '^': token.type = BOR; break;
                default: return token;
        }
        // incrementing pointer to compare the previous value identified by the type against the following
        token.type += 12; // set token.type to the first of the second order operations like ++ -- // so on
        current++; // increment current
        len = 2;
        // if you look at the types youll notice that its organized by groups of seven excluding == in the last group because == would be identifie already byt this first if and the thid order if would not be run
        // adding 7 to PLUS gives increments and adding 7 to EQUALS gives EQUALITY so on and so forth so we just check for if the current and next token are equal if so we dont change anything since 7 has already been
        // added otherwise we check if its an equal which adds another 7 so PLUS would become INCREMENT and with the presence of an = after it another 7 would be added giving PLUSEQUALS in the event of a +=
        // if none of those checks are entered than its a standalone value and we return the original result of the switch statement and change len to 1 for proper lexeme printing
        if (*current == '\0') return token;
        // starting with the simpler operation deciding weather
        if (c == *current) len = 2;
        // im too lazy right now to compact this logic so dont be suprised if its in the final copy
         else if (*current == '=') { len = 2;
                token.type += 12; // token.typeing to third order operators shouldnt overflow because == is the only outlier and its captured in the above statement
         } else {
                token.type -= 12; // decrement token.type to reconcile the uncoditional token.type
                len = 1;
        }
        // if we've made it to the end after all of that its a single operator
        token.type = token.type;
        if  (token.type == SLCOMMENT) {
                char *new = current;
                while (*new != '\n' && *new != '\0') {
                        len++;
                        new++;
                }
        }
        strncpy(token.lexeme, current - 1, len); // set the lexeme from current - 1 to adjust for incrementation earlier for check
        token.lexeme[len] = '\0';

        return token;
}

// this should be pretty self explanitory
Token seperator(char *current) {
        Token token = {UNKNOWN, "", 0, 0};
        // just a simple switch statement for finding weather current is a seperator nothing too special
        switch (*current) {
                case ';': token.type = SEMI; token.lexeme[0] = ';'; break;
                case ',': token.type = COMMA; token.lexeme[0] = ','; break;
                case '(': token.type = OPENP; token.lexeme[0] = '('; break;
                case ')': token.type = CLOSEP; token.lexeme[0] = ')'; break;
                case '{': token.type = OPENC; token.lexeme[0] = '{'; break;
                case '}': token.type = CLOSEC; token.lexeme[0] = '}'; break;
                case '[': token.type = OPENB; token.lexeme[0] = '['; break;
                case ']': token.type = CLOSEB; token.lexeme[0] = ']'; break;
                default: return token;
        }
        token.lexeme[1] = '\0';
        // update current becuase the main funtion does not
        return token;
}

// Identifier state machine see notes in DFA Images branch of this repository to see how I got this or just sift through the logic its not too complicated
Token identifier(char *current) {
        int index = 0, state = 1; // initial state
        Token token = {UNKNOWN, "", 0, 0};
        token.lexeme[0] = '\0';

        while (*current != '\0') {
                const char c = *current;
                if (!(((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c<= '9')) || c == '_')) break; // so we know when to end the loop

                switch (state) { // here the Φ state simply represents returning an UNKNOWN type to transition to this state we will simply set state to 0
                        case 0:
                                return token;
                        case 1:
                                if ((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || c == '_') state = 2; // set a as denoted in the regex to nfa ab*
                                else state = 0;
                                break;
                        case 2:
                                if (!((c >= 'a' && c <= 'z') || (c >= 'A' && c <= 'Z') || (c >= '0' && c<= '9') || c == '_')) state = 0;
                                break;
                }
                token.lexeme[index++] = c;
                current++;
        }
        token.lexeme[index] = '\0';
        token.type = (state == 2 ? IDENTIFIER : INVALID);
        return token;
}

// Number DFA Logic same thing as the Identifier state machine see DFA Images branch for notes or just logic your way through
Token number(char *current) {
        Token token = {UNKNOWN, "", 0, 0};

        int state = 1;
        int decicount = 0; // for Identifying weather its a decimal or integer and weather its a valid number without ending the loop early
                           // before I would end the loop if there were more than one decimal but than it would register 2.5.5 for example as 2. and then break come back and register the 5.5 as a valid number
                           // I wanted the lexeme to reflect the value that caused the error for better error handling
        int index = 0;

        while (*current != '\0') {
                const char c = *current;
                if (!((c >= '0' && c <= '9') || c == '.')) break; // if its not within set Σ end the loop and return

                switch (state) {
                        case 0:
                                token.type = INVALID;
                                break;
                        case 1:
                                if (c >= '0' && c <= '9') state = 2;
                                else state = 0;
                                break;
                        case 2:
                                if (!(c >= '0' && c <= '9')) state = 0;
                                if (c == '.' && decicount == 0) {
                                        state = 3;
                                        decicount++;
                                }
                                break;
                        case 3:
                                if (c >= '0' && c <= '9') state = 2;
                                else state = 0;
                                break;
                }
                token.lexeme[index++] = c;
                current++;
        }
        // if were here the format is valid
        token.lexeme[index] = '\0';
        if (state == 2) { // checking if it ended on a valid state
                token.type = decicount > 1 ? UNKNOWN : (decicount == 0) ? INT : DOUBLE;
        }
        return token;
}

// string literal DFA once again check DFA Images for notes on how I came up with this it would also help to read the lexical analysis test file for more info or just logic through it
Token string(char *current) {
        Token token = { STRING, "", 0, 0 };
        int state = 1;
        int index = 0;

        while (*current != '\0') {
                if (state == 5) break;
                const char c = *current;
                switch (state) {
                        case 0: token.type = INVALID; break;
                        case 1: if (c == '"') state = 2; else state = 0; break;
                        case 2: if (c == '"') state = 5; else if (c == '\\') state = 3; break;
                        case 3: if (!(c == '"' || c == '\\')) state = 0; else state = 2; break;
                        case 4: if (c == '"') state = 5; else state = 0; break;
                        case 5: break;
                }
                token.lexeme[index++] = c;
                current++;
        }
        token.lexeme[index] = '\0';
        return token;
}

// Main lexing method
Token *lexer_main(char *input) {
        Token token = {UNKNOWN, "", 0, 0};
        Token *tokens = malloc(sizeof(Token) * TOKEN_STREAM_LEN);
        TrieNode *root = createNode("", UNKNOWN);

        // inserting all of our keywords into the trie adds some overhead but imporves string comparison time for large wordsets which i plan on making this could be compacted with a keyword string array that maps
        // directly to the order of keywords in the typedef and in a for loop doing     insert(&root, keyWordStrings[i], i)     but I dont feel like doing that right now maybe once this list gets a little bigger
        insert(&root, "if", IF);
        insert(&root, "else", ELSE);
        insert(&root, "void", VOID);
        insert(&root, "case", CASE);
        insert(&root, "default", DEFAULT);
        insert(&root, "break", BREAK);
        insert(&root, "continue", CONTINUE);
        insert(&root, "while", WHILE);
        insert(&root, "for", FOR);
        insert(&root, "do", DO);
        insert(&root, "return", RETURN);
        insert(&root, "const", CONST);
        insert(&root, "let", LET);
        insert(&root, "var", VAR);
        insert(&root, "define", DEFINE);
        insert(&root, "class", CLASS);
        insert(&root, "class", CLASS);
        insert(&root, "object", OBJECT);
        insert(&root, "this", THIS);
        insert(&root, "super", SUPER);
        insert(&root, "extends", EXTENDS);
        insert(&root, "implements", IMPLEMENTS);
        insert(&root, "interface", INTERFACE);
        insert(&root, "try", TRY);
        insert(&root, "catch", CATCH);
        insert(&root, "finally", FINALLY);
        insert(&root, "throw", THROW);
        insert(&root, "public", PUBLIC);
        insert(&root, "private", PRIVATE);
        insert(&root, "protected", PROTECTED);
        insert(&root, "static", STATIC);
        insert(&root, "final", FINAL);
        insert(&root, "import", IMPORT);
        insert(&root, "export", EXPORT);
        insert(&root, "async", ASYNC);
        insert(&root, "await", AWAIT);

        // actual logic down here
        if (!tokens) { // just error handling
                perror("Error initializing token length");
                return NULL;
        }

        // initializing variables
        size_t cap = TOKEN_STREAM_LEN;
        size_t index = 0, line = 1, col = 1;
        char *current = input;

        printf("CURRENT INITIAL : %s", current);

        // start of the while loop
        while (*current != '\0') {
                token.type = UNKNOWN;
                token.lexeme[0] = '\0';

                // skip over whitespace
                while (isspace(*current)) {
                        if (*current == '\n') {
                                col = 0;
                                line++;
                        }
                        current++;
                        col++;
                }
                printf("CURRENT CHAR : %c\n", *current);

                // Tokenizing Logic calls
                if (*current == '"') {
                        // error handling for invalid strings
                        token = string(current);
                        if (token.type == INVALID) fprintf(stderr, "LEXICAL ERROR: Invalid escape attempt : '%s' \nline : %lu, col : %lu\n", token.lexeme, line, col);
                        if (*current == '\0') fprintf(stderr, "LEXICAL ERROR: reached end of the file while parsing string : '%s' \nline : %lu, col %lu\n", token.lexeme, line, col);
                }
                // calling the functions only overwriting if not previously Identified otherwise a valid token could be made unknown and an invalid meant for throwing errors could become unknown
                if (token.type == UNKNOWN) token = operator(current);
                if (token.type == UNKNOWN) token = seperator(current);
                if (token.type == UNKNOWN) token = number(current);
                if (token.type == UNKNOWN) token = identifier(current);
                if (token.type == IDENTIFIER) token.type = search(root, token.lexeme);
                if (token.type == UNKNOWN) current++;

                if (token.type == INVALID) {
                        // error handling if it makes it through all of that than the token is unrecognized
                        fprintf(stderr, "LEXICAL ERROR : Unrecognized Token : '%s' \nLINE : %lu, COL : %lu\n", token.lexeme, line, col);
                }
                // resize token array if capacity is exceeded this is called a dynamic array
                if (index >= cap) {
                size_t new_cap = cap * 2;
                Token *newTokens = realloc(tokens, sizeof(Token) * new_cap);
                        if (!newTokens) {
                                perror("Error reallocating tokens");
                                free(tokens);
                                return NULL;
                        }
                        tokens = newTokens;
                        cap = new_cap; // Update capacity
                }

                // current pointer incrementation
                size_t len = strlen(token.lexeme);
                if (len == 0) { token.lexeme[0] = *current; current++; }
                else current += len;

                // updating column and line in the token for error messages
                token.line = line;
                token.col = col;

                // debugging print and array adding
                printf("TOKEN AFTER: Type : %d, Lexeme : \'%s\', Line : %lu, Col : %lu\n\n", token.type, token.lexeme, line, col);
                tokens[index++] = token;
                token.type = UNKNOWN;
                token.lexeme[0] = '\0';
        }
        Token *finalTokens = realloc(tokens, sizeof(Token) * index-1);
        if (finalTokens) {
                free(tokens);
                tokens = finalTokens;
        }
        return tokens;
}
// reading the file into a buffer with error handling and passing the entire file as a self containe string within the program
// to the main lexer method just for ease of navigation instead of being forced to reference the characters via fgetc we can
// simply navigate as if it was a string contained within the program
int main() {
        // Dynamic memory allocation for input
        char *input = NULL;
        size_t len = 0;
        size_t read;

        // Open the file for reading
        FILE *file = fopen("test.unn", "r");
        if (!file) {
                perror("Could not open file");
                return 1; // Exit if the file cannot be opened
        }

        // Read the entire file into a buffer
        fseek(file, 0, SEEK_END); // Go to the end of the file
        len = ftell(file); // Get the file size
        fseek(file, 0, SEEK_SET); // Go back to the start of the file

        input = malloc(len + 1); // Allocate memory for the input
        if (!input) {
                perror("Failed to allocate memory");
                fclose(file);
                return 1; // Exit if memory allocation fails
        }

        read = fread(input, 1, len, file); // Read the file into the buffer
        input[read] = '\0'; // Null-terminate the string

        fclose(file); // Close the file
        lexer_main(input); // run operations on text with buffer
        free(input); // Free allocated memory

        return 0;
}
