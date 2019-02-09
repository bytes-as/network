#include <stdio.h>
#include <string.h>
#include <stdlib.h>

int main()
{
    char *str = (char *)malloc(100 * sizeof(char));
    strcpy(str, "This is me but not you just kidding ");

    // Returns first token
    char** token = (char **)malloc(100 * sizeof(char *));
    int i=0;
    token[i] = strtok(str, " ");
    // Keep printing tokens while one of the
    // delimiters present in str[].
    while (token[i] != NULL) {
      printf("%s\n", token[i]);
        i++;
        token[i] = strtok(NULL, " ");
      // long int convert = strtol(token[i], NULL, 0);
      // printf("this is the converted value : %ld\n", convert);
    }
    // printf("%d\n", i);
    return 0;
}
