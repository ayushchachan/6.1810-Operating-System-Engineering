#include <stdio.h>
#include <stdlib.h>
#include <fcntl.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    char *args[] = {"cat", NULL};
    if (fork() == 0) {
        close(0);
        open("input.txt", O_RDONLY);
        execvp("/bin/cat", args);
    } else {
        wait(NULL);
        char *argv[2];

        argv[0] = "wc";
        argv[1] = NULL;

        // close(0);
        // open("input.txt", O_RDONLY);

        execvp("wc", argv);
    }
}