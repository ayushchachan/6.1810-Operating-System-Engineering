#include <stdio.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>

int main(int argc, char *argv[])
{
    if (argc != 3)
    {
        printf("Incorrect number of arguments\n");
        return 1;
    }

    int rc = fork();

    if (rc < 0)
    {
        fprintf(stderr, "fork failed!\n");
        exit(1);
    }
    else if (rc == 0)
    {
        printf("I am a child process with pid =%d\n", getpid());
        execvp(argv[1], &argv[1]);
        fprintf(stderr, "exec '%s' failed: %s\n", argv[1], strerror(errno));
        _exit(EXIT_FAILURE);
    }
    else
    {
        int status;
        pid_t result = waitpid(rc, &status, 0);
        printf("waited for child with pid =%d\n", result);

        if (result < 0)
        {
            perror("waitpid");
            return 1;
        }

        if (WIFEXITED(status))
        {
            int exitcode = WEXITSTATUS(status);
            printf("exitcode = %d\n", exitcode);
            if (exitcode == 0)
            {
                printf("Command successfully completed\n");
            }
            else
            {
                return 1;
            }
        }
        else if (WIFSIGNALED(status))
        {
            fprintf(stderr, "child terminated by signal %d\n", WTERMSIG(status));
            return 1;
        }
        else
        {
            return 1;
        }
    }
    printf("exiting with status = 0\n");
    return 0;
}