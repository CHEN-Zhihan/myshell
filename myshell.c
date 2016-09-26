#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1026
#define MAX_CMD_NUMBER 5
#define VALID 0
typedef struct Command {
	int valid;
	int argc;
	char * argv[MAX_ARG_NUMBER];
	Command *next;
} Command;

ps
ls | ps

typedef struct Line {
	int type;//built-in or not
	int valid;
	int background;
	Command *head;
} Line;



int main(int argc, char const *argv[]) {
	char buffer[BUFFER_SIZE];
	while (true) {
		fprintf(stdout, "## myshell $ ");
		int error = get_command(buffer);//get the row command.
		if (error < 0) {
			report_input_error(error);
		} else {
			char * input = strndup(buffer,strlen(buffer) - 1);
			Line * line = parse(input);
			if (line.valid==VALID) {
				execute(line);
			} else {
				report_parse_error(line.valid);
			}
			free(input);
		}
	}
	return 0;
}