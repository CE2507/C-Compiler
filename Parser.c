#include <stdio.h> // standard i/o lib for C
#include <stdlib.h> // standard lib for C
#include <ctype.h> // lib for char type functions like isdigit()
#include <string.h> // lib for string functions like strcmp()

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