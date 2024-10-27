#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/wait.h>

#define MAX_INPUT 1024
#define MAX_ARGS 100

char error_message[30] = "An error has occurred\n";

void print_error()
{
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void handle_exit(char **args)
{
    if (args[1] != NULL) {
        print_error();
        return;
    }
    exit(0);
}

void handle_path(char **args)
{
}

void handle_cd(char **args)
{
    if (args[1] == NULL || args[2] != NULL)
    {
        print_error();
    }
    else if (chdir(args[1]) != 0)
    {
        print_error();
    }
}
void execute_external_command(char **args)
{
    if (fork() == 0)
    {                          
        execvp(args[0], args); 
        print_error();         
        exit(1);
    }
    else
    { 
        int status;
        waitpid(-1, &status, 0); 
    }
}

void execute_command(char *input)
{
    char *args[MAX_ARGS];
    char *token = strtok(input, " \t\n");
    int i = 0;
    while (token != NULL)
    {
        args[i++] = token;
        token = strtok(NULL, " \t\n");
    }
    args[i] = NULL;
    if (args[0] == NULL)
        return;
    if (strcmp(args[0], "exit") == 0)
    {
        handle_exit(args);
    }
    else if (strcmp(args[0], "cd") == 0)
    {
        handle_cd(args);
    }
    else if (strcmp(args[0], "path") == 0)
    {
        handle_path(args);
    }
    else
    {
        execute_external_command(args);
    }

}

void trim_newline(char *str)
{
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
    {
        str[len - 1] = '\0';
    }
}

int main(int argc, char *argv[])
{
    FILE *input = stdin;

    if (argc == 2)
    {
        input = fopen(argv[1], "r");
        if (input == NULL)
        {
            print_error();
            exit(1);
        }
    }
    else if (argc > 2)
    {
        write(STDERR_FILENO, error_message, strlen(error_message));
        exit(1);
    }
    char line[MAX_INPUT];
    while (1)
    {
        if (input == stdin)
            printf("wish> ");
        if (fgets(line, MAX_INPUT, input) == NULL)
            break;

        execute_command(line);
        trim_newline(line);

        if (strlen(line) == 0)
        {
            continue;
        }
    }
    fclose(input);
    exit(0);
}
