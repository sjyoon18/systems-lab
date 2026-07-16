#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 100
#define MAX_ARGS 10

void parse_args(char *cmd, char *argv[]) {
    int i = 0;
    char *token = strtok(cmd, " ");

    while (token != NULL && i < MAX_ARGS - 1) {
        argv[i] = token;
        i++;

        token = strtok(NULL, " ");
    }

    argv[i] = NULL;
}

void run_cmd(char *argv[]) {
    pid_t pid = fork();

    if(pid == 0) {
        execvp(argv[0], argv);
        perror("execvp");
        return;
    }

    wait(NULL);
}

void run_pipe(char *argv1[], char *argv2[]) {
    int fd[2];
    pipe(fd);

    pid_t pid1 = fork();

    if (pid1 == 0) {
        dup2(fd[1], 1);

        close(fd[0]);
        close(fd[1]);

        execvp(argv1[0], argv1);
        perror("execvp_1");
        return;
    }

    pid_t pid2 = fork();

    if (pid2 == 0) {
        dup2(fd[0], 0);

        close(fd[0]);
        close(fd[1]);

        execvp(argv2[0], argv2);
        perror("execvp_2");
        return;
    }

    close(fd[0]);
    close(fd[1]);

    wait(NULL);
    wait(NULL);
}

int main() {

    while(1) {
    
        char input[MAX_INPUT];

        printf("myshell> ");

        fgets(input, sizeof(input), stdin);

        input[strcspn(input, "\n")] = '\0';

        if (strlen(input) == 0) {
            continue;
        }

        char *pipe_pos = strchr(input, '|');

        if (pipe_pos == NULL) {
            char *argv[MAX_ARGS];

            parse_args(input, argv);

            if (argv[0] == NULL) {
                continue;
            }

            run_cmd(argv);
        } else {
            *pipe_pos = '\0';

            char *left = input;
            char *right = pipe_pos + 1;

            char *argv1[MAX_ARGS];
            char *argv2[MAX_ARGS];

            parse_args(left, argv1);
            parse_args(right, argv2);

            if (argv1[0] == NULL || argv2[0] == NULL) {
                continue;
            }

            run_pipe(argv1, argv2);
        }
    }

    return 0;
}