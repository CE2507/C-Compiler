#include <stdio.h> // standard input output library for C
#include <stdlib.h> // standard library for C
#include <ctype.h> // library for functions evaluating types such as isspace()
#include <string.h> // library for strings like strcmp() that compares strings
#include <regex.h> // library to use regex fuctions for compiling and execution etc...

#define MAX_TOKEN_LENGTH 128 // Limits the length of any given lexeme

// for more information on Lexical Analysis and Regex see Lexical analysis text file

typedef enum {
        IF, ELSE, WHILE, RETURN, VOID, FOR, BREAK, CONTINUE, SWITCH, CASE,
        IDENTIFIER,
        INT, DOUBLE, STRING,
        PLUS, MINUS, TIMES, DIVIDE, MOD, EQUALS, EQUALITY, NOT_EQUAL, LESS, GREATER, INCREMENT, DECREMENT, COMP_ADD, COMP_MINUS,
        OPENP, CLOSEP, OPENB, CLOSEB, OPENC, CLOSEC, SEMICOLON, COLON, COMMA, QUOTE, DQUOTE,
        SLCOMMENT, MLCOMMENT,
        INVALID,
} Type;

typedef struct {
        Type type;
        char lexeme[MAX_TOKEN_LENGTH];
        size_t line;
        size_t col;
} Token;

void printToken(Token token) {
        const char* typeNames[] = {
                "IF", "ELSE", "WHILE", "RETURN", "VOID", "FOR", "BREAK", "CONTINUE", "SWITCH", "CASE",
                "IDENTIFIER", "INT", "DOUBLE", "STRING", "PLUS", "MINUS", "TIMES", "DIVIDE", "MOD", "EQUALS",
                "EQUALITY", "NOT_EQUAL", "LESS", "GREATER", "INCREMENT", "DECREMENT", "COMP_ADD",
                "COMP_MINUS", "OPENP", "CLOSEP", "OPENB", "CLOSEB", "OPENC", "CLOSEC", "SEMICOLON",
                "COLON", "COMMA", "QUOTE", "DQUOTE", "SLCOMMENT", "MLCOMMENT", "INVALID"
        };

        if (token.type < INVALID) {
                printf("Token: %s, Lexeme: %s, Line: %zu, Column: %zu\n", typeNames[token.type], token.lexeme, token.line, token.col);
        } else {
                fprintf(stderr, "Error: Invalid token '%s' at Line: %zu, Column: %zu\n", token.lexeme, token.line, token.col);
        }
}

void tokenize(char* input, FILE *file) {
        regex_t regex[9]; // Array to hold compiled regex for each token type
        size_t line = 1;
        size_t col = 1;
        size_t len = 0;

        char* patterns[] = {
                "^(if|else|while|return|void|for|break|continue|switch|case)\\b", // Keywords
                "^[a-zA-Z_][a-zA-Z0-9_]*", // Identifiers
                "^[0-9]+(\\.[0-9]+)?", // Numbers: Integers and Doubles
                "^\\+\\=|\\-\\=|\\*\\=|\\/\\=", // Compound assignment operators (change from lexical analysis explanation file using or operation into concatenation of =)
                "^\\+\\+|--|\\+|\\-|\\*|\\/|\\=\\=|\\=", // Operators
                "^[;,.{}()]", // Seperators
                "^\"([^\"\\\\]|\\\\.)*\"", // String Literals
                "^//[^\n]*", // Single-line comments
                "^/\\*([^*]|[\\r\\n]|(\\*+[^*/]))*\\*+/", // Multi-line comments
        };

        // Compile all regex patterns once
        for (size_t i = 0; i < sizeof(patterns) / sizeof(patterns[0]); i++) {
                if (regcomp(&regex[i], patterns[i], REG_EXTENDED) != 0) {
                        fprintf(stderr, "Could not compile regex\n");
                        exit(1);
                }
        }

        char* current = input; // Pointer to traverse the input string

        // Loop through each character in the input
        while (*current != '\0') {
                // Skipping whitespace
                while (isspace(*current)) {
                        if (*current == '\n') line++;
                        current++; // Increment pointer to move to next character
                }
                Token token; // Initializing token
                token.lexeme[0] = '\0'; // Null-terminate lexeme
                token.line = line;
                token.col = col;

                // Loop through each regex pattern
                for (size_t i = 0; i < sizeof(regex) / sizeof(regex[0]); i++) {
                        regmatch_t match; // Array to store match positions
                        // Execute regex
                        if (regexec(&regex[i], current, 1, &match, 0) == 0 && match.rm_so == 0) {
                                // Set token lexeme to the matched portion of the string
                                int length = match.rm_eo - match.rm_so; // Length of the match
                                strncpy(token.lexeme, current, length); // Copy match to lexeme
                                token.lexeme[length] = '\0'; // Null-terminate to make sure no extra data from memory is printed see C pointers intro in Lexical Analysis text file for more

                                // Set token type based on index of matched regex in pattern
                                switch (i) { //
                                        case 0: // Keyword match
                                                if (strcmp(token.lexeme, "if") == 0) token.type = IF;
                                                else if (strcmp(token.lexeme, "else") == 0) token.type = ELSE;
                                                else if (strcmp(token.lexeme, "while") == 0) token.type = WHILE;
                                                else if (strcmp(token.lexeme, "return") == 0) token.type = RETURN;
                                                else if (strcmp(token.lexeme, "void") == 0) token.type = VOID;
                                                else if (strcmp(token.lexeme, "for") == 0) token.type = FOR;
                                                else if (strcmp(token.lexeme, "break") == 0) token.type = BREAK;
                                                else if (strcmp(token.lexeme, "continue") == 0) token.type = CONTINUE;
                                                else if (strcmp(token.lexeme, "switch") == 0) token.type = SWITCH;
                                                else if (strcmp(token.lexeme, "case") == 0) token.type = CASE;
                                                break;
                                        case 1: // Identifier match
                                                token.type = IDENTIFIER;
                                                break;
                                        case 2: // Number match
                                                if (strchr(token.lexeme, '.')) {
                                                        token.type = DOUBLE; // Double if it contains a decimal
                                                } else {
                                                        token.type = INT; // Integer otherwise
                                                }
                                                break;
                                        case 3: // // Compound assignment operators
                                                switch (token.lexeme[0]) {
                                                        case '+': token.type = COMP_ADD; break;
                                                        case '-': token.type = COMP_MINUS; break;
                                                }
                                                break;
                                        case 4: // Operators
                                                switch (token.lexeme[0]) {
                                                        case '+':
                                                                token.type = (token.lexeme[1] == '+') ? INCREMENT : PLUS;
                                                                break;
                                                        case '-':
                                                                token.type = (token.lexeme[1] == '-') ? DECREMENT : MINUS;
                                                                break;
                                                        case '*': token.type = TIMES; break;
                                                        case '/': token.type = DIVIDE; break;
                                                        case '=': token.type = EQUALS;
                                                                token.type = (token.lexeme[1] == '=') ? EQUALITY : EQUALS;
                                                                break;
                                                }
                                                break;
                                        case 5: // Seperators
                                                switch (token.lexeme[0]) {
                                                        case '(': token.type = OPENP; break;
                                                        case ')': token.type = CLOSEP; break;
                                                        case '{': token.type = OPENB; break;
                                                        case '}': token.type = CLOSEB; break;
                                                        case ';': token.type = SEMICOLON; break;
                                                        case ',': token.type = COMMA; break;
                                                }
                                                break;
                                        case 6: // String literal match
                                                token.type = STRING; // Simplified for string literals
                                                break;
                                        case 7: // Single-line comment match
                                                token.type = SLCOMMENT;
                                                break;
                                        case 8: // Multi-line comment match
                                                token.type = MLCOMMENT;
                                                break;
                                }

                                // Move the current pointer to the end of the matched string
                                col += length;
                                current += length;
                                printToken(token); // Print the token
                                break; // Exit loop once a match is found
                        }
                }
                // If no valid token was found
                if (token.lexeme[0] == '\0') {
                        token.type = INVALID; // Set type to INVALID
                        token.lexeme[0] = *current; // Set lexeme to the current character
                        token.lexeme[1] = '\0'; // Null terminate
                        fprintf(stderr, "Error: Unrecognized token '%c' at Line: %zu, Column: %zu\n", *current, line, col);
                        current++; // Move to the next character
                        col++;
                }
        }

        // Free compiled regex
        for (size_t i = 0; i < sizeof(regex) / sizeof(regex[0]); i++) {
                regfree(&regex[i]);
        }
}

int main() {
        // Dynamic memory allocation for input
        char *input = NULL;
        size_t len = 0;
        ssize_t read;

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
        tokenize(input, NULL); // Tokenize the input read from the file
        free(input); // Free allocated memory

        return 0;
}