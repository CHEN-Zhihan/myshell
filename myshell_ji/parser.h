#ifndef PARSER_H_
#define  PARSER_H_




#define MAX_COMMAND_SIZE 1026 // Since fgets will add '\0' and the last char is '\n', we need to use 1026 instead of 1024
#define MAX_TOKEN_NUMBER 30 //max number of token as specified in assignment2


#define format_buffer(x) buffer[strlen(buffer) - 1] = 0 //delete the '\n' at the end of the command


#define GET_RESULT 1
#define NO_RESULT 0



int get_command(char *buffer);

struct Command *parse_cmd(char *input, int *is_background);
struct Command *parse_pipe(char *input);
struct Command *parse_exec(char *input);

struct Command *make_pipe_from_toke(char **token, int i, int number);
int split_input(char *input, char **output, char *delimiter, int flag);
int countchar(char* input, int delimiter);
int allspace(char *s);
int background(char *buffer);



#endif
