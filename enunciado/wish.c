#include <stdio.h>
#include <stdlib.h>

int main() {
    char *line = NULL;
    size_t len = 0;

    while (1) {
        printf("wish> ");  
        if (getline(&line, &len, stdin) == -1) {
            break;  
        }
        printf("Comando ingresado: %s", line);
    }

    free(line);  
    return 0;
}