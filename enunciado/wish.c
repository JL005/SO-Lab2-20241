#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <sys/types.h>
#include <signal.h>
#include <sys/wait.h>
#include <fcntl.h>
#include <termios.h>

#define MAX_INPUT 1024
#define MAX_ARGS 100

char error_message[30] = "An error has occurred\n";

char *shell_path[MAX_ARGS];
int path_count = 0;

void print_error()
{
    write(STDERR_FILENO, error_message, strlen(error_message));
}

void trim_newline(char *str)
{
    size_t len = strlen(str);
    if (len > 0 && str[len - 1] == '\n')
    {
        str[len - 1] = '\0';
    }
}

void handle_exit(char **args)
{
    if (args[1] != NULL)
    {
        print_error();
        return;
    }
    exit(0);
}

void handle_path(char **args)
{
    if (args == NULL)
    {
        print_error();
        return;
    }
    path_count = 0;
    for (int i = 1; args[i] != NULL; i++)
    {
        shell_path[path_count++] = strdup(args[i]); // Guarda el directorio
    }
    shell_path[path_count] = NULL; // Marca el final del path
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
    else
    {
        char cwd[MAX_INPUT];
        if (getcwd(cwd, sizeof(cwd)) != NULL)
        {
        }
        else
        {
            print_error();
        }
    }
}

char *find_command(char *command)
{
    static char full_path[MAX_INPUT];
    for (int i = 0; i < path_count; i++)
    {
        snprintf(full_path, sizeof(full_path), "%s/%s", shell_path[i], command);
        if (access(full_path, X_OK) == 0)
        {
            return full_path;
        }
    }
    return NULL;
}

int redirect_output(char *filename)
{
    int fd = open(filename, O_WRONLY | O_CREAT | O_TRUNC, S_IRWXU);
    if (fd < 0)
    {
        printf("error: %s\n", filename);

        print_error();
        return -1;
    }
    dup2(fd, STDOUT_FILENO);
    dup2(fd, STDERR_FILENO);
    close(fd);
    return 0;
}

void execute_external_command(char **args, char *redirect_file)
{
    char *command_path = find_command(args[0]);

    if (command_path == NULL)
    {

        print_error();
        return;
    }
    if (fork() == 0)
    {
        if (redirect_file != NULL)
        {
            if (redirect_output(redirect_file) < 0)
            {
                exit(1);
            }
        }
        execvp(command_path, args);
        perror(args[0]);
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
    char *redirect_file = NULL;
    int i = 0;
    char *command = strtok(input, ">");
    char *file = strtok(NULL, ">");
    if (file != NULL)
    {
        trim_newline(file);
        char *filename = strtok(file, " \t");
        char *extra = strtok(NULL, " \t");
        if (filename == NULL || extra != NULL)
        {
            print_error();
            return;
        }
        /*if (strlen(file) == 0)
        {
            print_error();
            return;
        }*/
        redirect_file = strtok(file, " \t");
    }
    char *token = strtok(command, " \t\n");
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
        execute_external_command(args, redirect_file);
    }
}

void initialize_path()
{
    shell_path[0] = strdup("/bin");
    shell_path[1] = strdup("/usr/bin");
    path_count = 2;
    shell_path[2] = NULL;
}

int main(int argc, char *argv[])
{
    FILE *input = stdin;
    initialize_path();
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
