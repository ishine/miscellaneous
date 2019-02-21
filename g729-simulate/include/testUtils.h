#ifndef TEST_UTILS_H
#define TEST_UTILS_H

void printUsage(char *command);
int getArgument(int argc, char *argv[],  char** filePrefix);
int getArgumentsMultiChannel(int argc, char *argv[], char *filePrefix[]);

#endif
