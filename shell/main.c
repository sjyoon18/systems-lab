#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>

#define MAX_INPUT 100
#define MAX_ARGS 10

int main() {

    while(1) {

        char input[MAX_INPUT];
        char *argv[MAX_ARGS];

        printf("myshell> ");

        fgets(input, sizeof(input), stdin);

        input[strcspn(input, "\n")] = '\0';

        int i = 0;

        char *token = strtok(input, " ");

        while(token != NULL && i < MAX_ARGS - 1) {
            argv[i] = token;
            i++;

            token = strtok(NULL, " ");
        }

        if (argv[0] == NULL) {
            continue;
        }

        pid_t pid = fork();

        if (pid == 0) {
            execvp(argv[0], argv);

            perror("execvp");
            return 1;
        }

        wait(NULL);
    }

    return 0;
}