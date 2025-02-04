// ish: Irfan's Shell
// Modified from: https://brennan.io/2015/01/16/write-a-shell-in-c/

#include <sys/wait.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// Function declarations for builtin shell commands:
int ish_cd(char** args);
int ish_help(char** args);
int ish_exit(char** args);

// List of builtin commands, followed by their corresponding functions
char* builtin_str[] = {
    "cd",
    "help",
    "exit"
};

int (*builtin_func[]) (char**) = {
    &ish_cd,
    &ish_help,
    &ish_exit
};

int ish_num_builtins() {
    return sizeof(builtin_str) / sizeof(char *);
}

// Builtin function implementations
int ish_cd(char** args)
{
    if (args[1] == NULL) {
        fprintf(stderr, "ish: Expected argument to \"cd\"\n");
    } else {
        if (chdir(args[1]) != 0) {
            perror("ish");
        }
    }

    return 1;
}

int ish_help(char **args)
{
    int i;
    printf("Irfan's Shell - ISH\n");
    printf("Type program names and arguments, and hit enter.\n");
    printf("The following are built in:\n");

    for (i = 0; i < ish_num_builtins(); i++) {
        printf("  %s\n", builtin_str[i]);
    }

    printf("Use the man command for information on other programs.\n");
    return 1;
}

int ish_exit(char **args)
{
    return 0;
}

int ish_launch(char** args)
{
    pid_t pid, wpid;
    int status;

    pid = fork();
    if (pid == 0) {
        // Child process
        if (execvp(args[0], args) == -1) {
            perror("ish");
        }
        exit(EXIT_FAILURE);
    } else if (pid < 0) {
        // Error forking
        perror("ish");
    } else {
        // Parent process
        do {
            wpid = waitpid(pid, &status, WUNTRACED);
        } while (!WIFEXITED(status) && !WIFSIGNALED(status));
    }

    // Program should never reach this, unless there's an error
    return 1;
}

int ish_execute(char **args)
{
  int i;

  if (args[0] == NULL) {
    // An empty command was entered.
    return 1;
  }

  for (i = 0; i < ish_num_builtins(); i++) {
    if (strcmp(args[0], builtin_str[i]) == 0) {
      return (*builtin_func[i])(args);
    }
  }

  return ish_launch(args);
}

#define ISH_TOK_BUFSIZE 64
#define ISH_TOK_DELIM " \t\r\n\a"
char** ish_split_line(char *line)
{
    int bufsize = ISH_TOK_BUFSIZE, position = 0;
    char** tokens = malloc(bufsize * sizeof(char *));
    char* token;

    if (!tokens) {
        fprintf(stderr, "ish: There was a problem allocating memory for the tokens. Maybe try closing some running programs or processes?\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, ISH_TOK_DELIM);
    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= bufsize) {
            bufsize += ISH_TOK_BUFSIZE;
            tokens = realloc(tokens, bufsize * sizeof(char *));
            if (!tokens) {
                fprintf(stderr, "ish: There was a problem re-allocating memory for the tokens. Maybe try closing some running programs or processes?\n");
                exit(EXIT_FAILURE);
            }
        }

        token = strtok(NULL, ISH_TOK_DELIM);
    }
    tokens[position] = NULL;
    return tokens;
}

#define ISH_RL_BUFSIZE 1024
char* ish_read_line(void)
{
    int bufsize = ISH_RL_BUFSIZE;
    int position = 0;
    char* buffer = malloc(sizeof(char) * bufsize);
    int c;

    if (!buffer) {
        fprintf(stderr, "ish: There was a problem allocating memory for the buffer storing your input. Maybe try closing some running programs or processes?\n");
        exit(EXIT_FAILURE);
    }

    while (1) {
        // Read a character
        c = getchar();
        
        // If we hit EOF, replace it with a null character and return
        if (c == EOF || c == '\n') {
            buffer[position] = '\0';
            return buffer;
        } else {
            buffer[position] = c;
        }
        position++;

        // If we exceed the buffer, reallocate
        if (position >= bufsize) {
            bufsize += ISH_RL_BUFSIZE;
            buffer = realloc(buffer, bufsize);
            if (!buffer) {
                fprintf(stderr, "ish: There was a problem re-allocating memory for the buffer storing your input. Maybe try closing some running programs or processes?\n");
                exit(EXIT_FAILURE);
            }
        }
    }
}

void ish_loop(void)
{
    char* line;
    char** args;
    int status;

    do {
        printf("ish %% ");
        line = ish_read_line();
        args = ish_split_line(line);
        status = ish_execute(args);

        free(line);
        free(args);
    } while (status);
}

int main(int argc, char** argv)
{
    // Load config files

    // Run command loop
    ish_loop();

    // Perform any cleanup / shutdown
    
    return EXIT_SUCCESS;
}