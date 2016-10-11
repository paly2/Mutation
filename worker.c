#include <stdio.h>
#include <stdlib.h>
#include <signal.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/stat.h>

#define BASE_FILE "program/base"
#define BASE_FILE_CPY "program/base_cpy"
#define CPY_BUFFER_SIZE 256

int current_child_pid;
FILE* copy_base(int* nb_bytes) {
	FILE* base = NULL;
	FILE* base_cpy = NULL;
	int ch = 0;
	size_t nb_read_bytes = 0;

	base = fopen(BASE_FILE, "r");
	if (base == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}
	base_cpy = fopen(BASE_FILE_CPY, "w");
	if (base_cpy == NULL) {
		perror("fopen");
		exit(EXIT_FAILURE);
	}

	*nb_bytes = 0;
	fseek(base, 0, SEEK_SET);
	fseek(base_cpy, 0, SEEK_SET);
	while ((ch = fgetc(base)) != EOF) {
		fputc(ch, base_cpy);
		(*nb_bytes)++;
	}

	fclose(base);

	chmod(BASE_FILE_CPY, 0755);
	return base_cpy;
}

void kill_alarm(int sig) {
	kill(current_child_pid, SIGTERM);
}

int main(int argc, char* argv[]) {
	int nb_bytes;
	FILE* prog;
	int counter;
	pid_t pid;
	int status;

	srand(time(NULL));
	signal(SIGALRM, kill_alarm);
	for (counter = 1 ; ; counter++) {
		printf("Mutation n°: %d will now run\n", counter);

		/* Copy the base file (the mutation will probably result to
		 * death, we need to conserve a sain file). This is a very
		 * convenient function that also opens the copied file, and
		 * gives us the number of bytes in the program.
		 */
		prog = copy_base(&nb_bytes);

		/* Change a random byte to a random number in the program. This
		 * can add a byte to the end of the program.
		 */
		fseek(prog, rand() % (nb_bytes + 1), SEEK_SET);
		fputc(rand() % 256, prog);
		fclose(prog);

		printf("Program size: %d\n", nb_bytes);

		/* Fork off and run the new program.
		 */
		printf("=== mutating program zone ===\n");
		pid = fork();
		if (pid == 0) { // Child
			if (execl(BASE_FILE_CPY, NULL) == -1) {
				perror("excl");
				return EXIT_FAILURE;
			}
		} else { // Parent
			current_child_pid = pid;
			alarm(2); // The child can run two seconds before being killed.
			waitpid(current_child_pid, &status, 0);
			printf("=== mutating program zone end ===\n");
			if (WIFEXITED(status)) {
				if (WEXITSTATUS(status) == EXIT_FAILURE) {
					// Call to excl has probably failed.
					printf("Mutation n° %d has failed. Re-using the previous program.\n", counter);
				} else {
					// That's good ! The child exited (it was not killed) !
					printf("Mutation n° %d has succeeded. Continuing with the same program.\n", counter);
					// Replace the base program with this mutated copy
					remove(BASE_FILE);
					rename(BASE_FILE_CPY, BASE_FILE);
				}
			} else {
				// Bad, bad...
				if (WIFSIGNALED(status)) {
					printf("Mutating program has been terminated by signal: ");
					switch (WTERMSIG(status)) {
						case SIGTERM:
							printf("SIGTERM\n");
							break;
						case SIGINT:
							printf("SIGINT\n");
							break;
						case SIGKILL:
							printf("SIGKILL\n");
							break;
						case SIGSEGV:
							printf("SIGSEGV\n");
							break;
						default:
							printf("%d\n", WTERMSIG(status));
							break;
					}
				}
				printf("Mutation n° %d has failed. Re-using the previous program.\n", counter);
			}
		}
	}

	return EXIT_SUCCESS;
}

