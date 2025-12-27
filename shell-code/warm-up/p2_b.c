#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>

int main(void) {
    
    int rc = fork();

    if (rc < 0) {
        fprintf(stderr, "fork failed\n");
        exit(1);
    } else if (rc == 0) {
        execlp("ls", "ls", "-l", (char *)NULL);
        fprintf(stderr, "execlp failed!\n");
        exit(1);
    } else {
        int status;
        wait(&status);
    }
    return 0;
}