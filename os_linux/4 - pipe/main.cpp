#include <unistd.h>
#include <sys/wait.h>
#include <iostream>
#include <string>

int main() {
    int pipe1[2];
    int pipe2[2];
    int pipe3[2];

    pipe(pipe1);
    pipe(pipe2);
    pipe(pipe3);

    pid_t pidM = fork();
    if (pidM == 0) {
        dup2(pipe1[1], STDOUT_FILENO);
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        close(pipe3[0]);
        close(pipe3[1]);
        execl("./M", "M", NULL);
        return 1;
    }

    pid_t pidA = fork();
    if (pidA == 0) {
        dup2(pipe1[0], STDIN_FILENO);
        dup2(pipe2[1], STDOUT_FILENO);
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        close(pipe3[0]);
        close(pipe3[1]);
        execl("./A", "A", NULL);
        return 1;
    }

    pid_t pidP = fork();
    if (pidP == 0) {
        dup2(pipe2[0], STDIN_FILENO);
        dup2(pipe3[1], STDOUT_FILENO);
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        close(pipe3[0]);
        close(pipe3[1]);
        execl("./P", "P", NULL);
        return 1;
    }

    pid_t pidS = fork();
    if (pidS == 0) {
        dup2(pipe3[0], STDIN_FILENO);
        close(pipe1[0]);
        close(pipe1[1]);
        close(pipe2[0]);
        close(pipe2[1]);
        close(pipe3[0]);
        close(pipe3[1]);
        execl("./S", "S", NULL);
        return 1;
    }

    close(pipe1[0]);
    close(pipe1[1]);
    close(pipe2[0]);
    close(pipe2[1]);
    close(pipe3[0]);
    close(pipe3[1]);

    waitpid(pidM, NULL, 0);
    waitpid(pidA, NULL, 0);
    waitpid(pidP, NULL, 0);
    waitpid(pidS, NULL, 0);

    return 0;
}
