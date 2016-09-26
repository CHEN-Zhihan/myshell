#include <stdio.h>
#include <string.h>

#define BUFFER_SIZE 1026
#define MAX_CMD_NUMBER 5

typedef struct Command {
	int valid;
	int argc;
	char * argv[MAX_ARG_NUMBER];
} Command;



typedef struct Line {
	int type;
	int valid;
	int background;
	int commandNumber;
	Command cmds[MAX_CMD_NUMBER]
} Line;



int main(int argc, char const *argv[]) {
	char buffer[BUFFER_SIZE];
	while (true) {
		fprintf(stdout, "## myshell $ ");
		//很蛋疼啊，error应该在哪里处理，都在main里处理会不会把main搞得太复杂
		//还是每个function都搞一个warpper?
		int size = get_command(buffer);//get the row command.
		if (size<=1) {
			report_input_error(size);
		} else {
			char * line = strndup(buffer,--size);
			Command * cmd = parse(line);
			if (cmd.valid==0) {
				if (cmd.type==EXIT) {
					fprintf(stdout,"myshell: Terminated\n");
					free(line);
					return 0;
				} else {
					execute(cmd);
				}
			} else {
				report_parse_error(cmd.valid);
			}
			free(line);
		}
		//TODO::::::::::
		//1.handle the error and format the buffer.
		//2. determine whether is internal command
		//3. parse the command.
		//4. run the command.
	}
	return 0;
}