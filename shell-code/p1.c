#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/wait.h>

int main(void)
{
    int rc = fork();
    if (rc < 0)
    {
        fprintf(stderr, "Fork failed!\n");
        exit(1);
    }
    else if (rc == 0)
    {
        // child process
        printf("I am child, pid = %d\n", getpid());
        exit(2);
    }
    else
    {
        int status;
        wait(&status);
        printf("I am parent, pid = %d\n", getpid());
    }
}