#ifndef UTIL_H
#define UTIL_H
#include <string.h>
#include <stdlib.h>
#ifndef nullptr
#define nullptr NULL
#endif

#ifndef true
#define true 1
#endif

#ifndef false
#define false 0
#endif

#ifndef bool
#define bool int
#endif

bool allSpace(char *input);

int split_input(char *input, char **output, char *delimiter, bool flag);




#endif