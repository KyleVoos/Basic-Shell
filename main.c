#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>

void execCMD(char *path, char **args) {

}

void getArgs(char *input, char **args) {
    char *token;
    char *delim = " ";
//    char **args = (char **) malloc(sizeof(char *));
    int ii = 1;

    token = strtok(input, delim);
    fprintf(stdout, "token = %s\n", token);
    args[0] = strdup(token);
    while ((token = strtok(NULL, delim)) != NULL) {
        fprintf(stdout, "token = %s\n", token);
        args[ii] = strdup(token);
        ii++;
    }
}

char *findPath(char * arg) {
    char *path = getenv("PATH");
    char *token;
    char *delim = ":";
    struct stat s;
    char *execPath;

    fprintf(stdout, "PATH = %s\n", path);
    token = strtok(path, delim);
    while (token != NULL) {
        fprintf(stdout, "token = %s\n", token);
        execPath = strdup(token);
        strcat(execPath, "/");
        strcat(execPath, arg);
        fprintf(stdout, "execPath = %s\n", execPath);
        if (stat(execPath, &s) != -1) {
            fprintf(stdout, "%s is in %s\n",arg, execPath);
            return execPath;
        }
        token = strtok(NULL, delim);
    }
    execPath = "./";
    strcat(execPath, arg);
    if (stat(execPath, &s) != -1) {
        fprintf(stdout, "%s is in %s\n",arg, execPath);
        return execPath;
    }
    fprintf(stdout, "ERROR: %s not found!\n", arg);

    return NULL;
}

int main(int argc, char *argv[]) {
   char *path;
   char *line = NULL;
   size_t len = 0;
   char **args = (char **) malloc(sizeof(char *));

    if (argc > 1) {
        while (1) {
            getline(&line, &len, stdin);
            getArgs(line, args);
            if ((path = findPath(argv[1])) != NULL) {
                fprintf(stdout, "got here\n");
            }
        }
    }
    return 0;
}