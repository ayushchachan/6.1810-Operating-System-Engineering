#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define MAX_DIR_LENGTH 256
#define MAX_CMD_LENGTH 1024
#define MAX_ARG_SIZE 64
#define MAX_ARGS 64
#define MAX_BG_PROCESSES 64
#define MAX_FG_CMD 64

void eval(char *cmdline);
int parseline(char buf[], char *argv[]);
int builtin_command(char *argv[]);

int bg_process_count = 0;				// count of background process running
pid_t bg_pids[MAX_BG_PROCESSES];

int main(void) {
    char cmdline[MAX_CMD_LENGTH];
    char curr_dir[MAX_DIR_LENGTH];

    while (1) {
        /*print current directory*/
        if (getcwd(curr_dir, sizeof(curr_dir)) == NULL) {
            fprintf(stderr, "getcwd error: %s\n", strerror(errno));
            strcpy(curr_dir, "?");
        }
        printf("%s $ ", curr_dir);
        fflush(stdout);

        /*Begin taking input*/
        if (fgets(cmdline, sizeof(cmdline), stdin) == NULL) {
            // Ctrl+D
            printf("\n");
            break;
        }

        pid_t rc;                                       // reaped child
        while ((rc = waitpid(-1, NULL, WNOHANG)) > 0) {
            for (int i = 0; i < bg_process_count; i++) {
				if (bg_pids[i] == rc) {
					// swap with last pid
					bg_pids[i] = bg_pids[bg_process_count - 1];

                    bg_process_count--;
                    printf("Background process with pid = %d finished.\n", rc);
					break;
				}
			}
        }

        /*evaluate*/
        eval(cmdline);
    }
    return 0;
}

/*eval - Evaluate a command line*/
void eval(char *cmdline) {
    char *argv[MAX_ARGS];               // argument list for exec syscall
    char buf[MAX_CMD_LENGTH];
    int bg;                             // should the job run in bg and fg
    pid_t pid;                          // process id

    strcpy(buf, cmdline);
    bg = parseline(buf, argv);

    printf("command entered: %s", cmdline);

    // Empty command
    if (argv[0] == NULL) {
        return;
    }

    // Exit command
    if (strcmp(argv[0], "exit") == 0) {
        while (bg_process_count > 0) {
            kill(bg_pids[bg_process_count--], SIGTERM);
        }
        exit(0);
    }

    // support for cd command
    if (strcmp(argv[0], "cd") == 0) {
        if (argv[1] == NULL) {
            fprintf(stderr, "Usage: cd <directoryname>\n");
        } else if (chdir(argv[1]) != 0) {
            fprintf(stderr, "Error: %s %s\n", argv[1], strerror(errno));
        }
        return;
    }

    pid = fork();

    if (pid < 0) {
        fprintf(stderr, "fork error: %s\n", strerror(errno));
        exit(1);
    } else if (pid == 0) {
        execvp(argv[0], argv);
        fprintf(stderr, "exec failed: %s\n", strerror(errno));
        exit(1);
    } else {
        if (bg) {
            if (bg_process_count < MAX_BG_PROCESSES) {
                bg_pids[bg_process_count++] = pid;
                printf("%d %s", pid, cmdline);
            } else {
                fprintf(stderr, "Too many background processes");
            }
        } else {
            int status;
            waitpid(pid, &status, 0);
            if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
                printf("EXITSTATUS: %d\n", WEXITSTATUS(status));
            }
        }
    }

    return;
}



/*parseline - parse the command 'buf' and build the array of argument 'argv'*/
int parseline(char buf[], char *argv[]) {
    size_t len = strlen(buf);
    if (len > 0 && (buf[len - 1] == '\n')) {
        buf[len - 1] = ' ';
        len--;
    }

    /* skip leading spaces */
    while (*buf && (*buf == ' ')) {
        buf++;
    }

    int argc = 0;
    char *delim;

    while ((delim = strchr(buf, ' ')) != NULL) {
        argv[argc] = buf;
        argc++;
        *delim = '\0';

        buf = delim + 1;
        while (*buf && (*buf == ' ')) {
            buf++;
        }
    }
    argv[argc] = NULL;

    if (argc == 0) {
        return 0;
    }

    int bg = 0;             // background process

    if (strcmp(argv[argc - 1], "&") == 0) {
        bg = 1;
        argv[--argc] = NULL;
    }
    return bg;
}