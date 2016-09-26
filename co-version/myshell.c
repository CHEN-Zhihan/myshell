#include <stdio.h>

#define BUFFER_SIZE 1026


int main(int argc, char const *argv[]) {
	char buffer[BUFFER_SIZE];
	while (true) {
		fprintf(stdout, "## myshell $ ");
		int error = get_command(buffer);//get the row command.
		//TODO::::::::::
		//1.handle the error and format the buffer.
		//2. determine whether is internal command
		//3. parse the command.
		//4. run the command.
	}
	return 0;
}