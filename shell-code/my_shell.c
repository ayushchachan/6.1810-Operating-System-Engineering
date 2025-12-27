#include <stdio.h>
#include <sys/types.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <errno.h>
#include <signal.h>

#define MAX_DIR_SIZE 256
#define MAX_INPUT_SIZE 1024
#define MAX_TOKEN_SIZE 64
#define MAX_NUM_TOKENS 64
#define MAX_BG_PROCESSES 64
#define MAX_FG_CMD 64

void sigint_handler(int sig);
volatile sig_atomic_t fg_pid = 0;

/* Splits the string by space and returns the array of tokens
 *
 */
char **tokenize(char *line)
{
	char **tokens = (char **)malloc(MAX_NUM_TOKENS * sizeof(char *));
	char *token = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
	int i, tokenIndex = 0, tokenNo = 0;

	for (i = 0; i < strlen(line); i++)
	{

		char readChar = line[i];

		if (readChar == ' ' || readChar == '\n' || readChar == '\t')
		{
			token[tokenIndex] = '\0';
			if (tokenIndex != 0)
			{
				tokens[tokenNo] = (char *)malloc(MAX_TOKEN_SIZE * sizeof(char));
				strcpy(tokens[tokenNo++], token);
				tokenIndex = 0;
			}
		}
		else
		{
			token[tokenIndex++] = readChar;
		}
	}

	free(token);
	tokens[tokenNo] = NULL;
	return tokens;
}

void sigint_handler(int sig) {
	if (fg_pid > 0) {
		kill(fg_pid, SIGINT);
	}

}

int main(int argc, char *argv[])
{
	char cur_dir[MAX_DIR_SIZE];
	char line[MAX_INPUT_SIZE];
	char **tokens;
	int i;
	int count = 0;				// count of background process running
	pid_t bg_pids[MAX_BG_PROCESSES];

	signal(SIGINT, sigint_handler);

	while (1)
	{
		/* BEGIN: TAKING INPUT */
		getcwd(cur_dir, sizeof(cur_dir));
		printf("%s $ ", cur_dir);
		fflush(stdout);

		fg_pid = 0;
		
		if (fgets(line, sizeof(line), stdin) == NULL)
        {
            // Ctrl+D
            break;
        }

        tokens = tokenize(line);

		for (int i = 0; tokens[i] != NULL; i++) {
			printf("found token %s (remove this debug output later)\n", tokens[i]);
		}
		// printf("Command entered: %s (remove this debug output later)\n", line);
		/* END: TAKING INPUT */
		
		pid_t rc;
		while ((rc = waitpid(-1, NULL, WNOHANG)) > 0) {
			for (i = 0; i < count; i++) {
				if (bg_pids[i] == rc) {
					// swap with last pid
					pid_t temp = bg_pids[i];
					bg_pids[i] = bg_pids[count - 1];
					bg_pids[count - 1] = temp;
					break;
				}
			}
			count--;
			printf("Shell: Background process finished\n");
		}

		// Empty line
		if (tokens[0] == NULL) {
			free(tokens);
			continue;
		}

		// Exit command
		if (strcmp(tokens[0], "exit") == 0) {
			
			while (count > 0) {
				kill(bg_pids[count - 1], SIGTERM);
				count--;
			}

			// Freeing the allocated memory
			for (i = 0; tokens[i] != NULL; i++)
			{
				free(tokens[i]);
			}
			free(tokens);
			break;
		}

		// support for cd command
		if (strcmp(tokens[0], "cd") == 0) {
			if (tokens[1] == NULL) {
				fprintf(stderr, "Usage: cd <directoryname>\n");
			} else if (chdir(tokens[1]) != 0) {
				fprintf(stderr, "Error: %s %s\n", tokens[1], strerror(errno));
			}
			for (i = 0; tokens[i] != NULL; i++)
			{
				free(tokens[i]);
			}
			free(tokens);
			continue;

		}

		int num_tokens = 0;
		for (i = 0; tokens[i] != NULL; i++ ) {
			num_tokens++;
		}

		int is_background = 0;
		int is_serial = 0;
		int is_parallel = 0;

		for (int i = 0; tokens[i] != NULL; i++) {
			if ((tokens[i], "&&") == 0) {
				is_serial = 1;
				break;
			}
		}

		if (strcmp(tokens[num_tokens - 1], "&") == 0) {
			is_background = 1;
			tokens[num_tokens - 1] = NULL;
		}
		char *args[MAX_TOKEN_SIZE];
		int j = 0;								// an index into token
		for (i = 0; tokens[i] != NULL; i++) {
			args[j++] = tokens[i];
			if (strcmp(tokens[i], "&&") == 0 || strcmp(tokens[i], "&&&") == 0) {

			} else {
				continue;
			}
			pid_t pid = fork();

			if (!is_background) {
				fg_pid = pid;
			}

			if (pid == 0) {
				signal(SIGINT, SIG_DFL);
				if (is_background) {
					setpgid(0, 0);
				}
				execvp(tokens[0], tokens);
				fprintf(stderr, "exec failed: %s %s\n", tokens[0], strerror(errno));
				exit(1);
			} else if (pid > 0) {
				int status;

				if (is_background) {
					bg_pids[count] = pid;
					count++;
				} else {
					waitpid(pid, &status, 0);
					if (WIFEXITED(status) && WEXITSTATUS(status) != 0) {
						printf("EXITSTATUS: %d\n", WEXITSTATUS(status));
					}
				}
				
			} else {
				fprintf(stderr, "Error! fork failed: %s %s\n", tokens[0], strerror(errno));
			}
		}

		// Freeing the allocated memory
		for (i = 0; tokens[i] != NULL; i++)
		{
			free(tokens[i]);
		}
		free(tokens);
	}
	return 0;
}
