#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>

typedef struct _arguments {
    char *path;
    char **args;
    int numArgs;
    int noWait;
} arguments;

void execCMD(arguments arg) {
    pid_t pid;
    int wstatus;

    fprintf(stdout, "number of arguments = %d\n", arg.numArgs);

    if ((pid = fork()) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        return;
    }

    if (pid == 0) {
        execv(arg.path, arg.args);
    }
    else {
        if (arg.noWait == 0)
            wait(&wstatus);
        else {
            fprintf(stdout, "& detected\n");
            return;
        }
    }
}

void getArgs(char *input, arguments *arg) {
    char *token;
    char *delim = " ";
//    char **args = (char **) malloc(sizeof(char *));
    int ii = 1;

    token = strtok(input, delim);
    if (token[strlen(token) - 1] == '\n')
        token[strlen(token) - 1] = '\0';
    fprintf(stdout, "token = %s\n", token);
    arg->args[0] = strdup(token);
    fprintf(stdout, "args[0] = %s\n", arg->args[0]);
    while ((token = strtok(NULL, delim)) != NULL) {
        if (token[strlen(token) - 1] == '\n')
            token[strlen(token) - 1] = '\0';
        fprintf(stdout, "token = %s\n", token);
        if (strcmp(token, "&") == 0) {
            arg->noWait = 1;
        }
        else {
            arg->args[ii] = strdup(token);
            fprintf(stdout, "args[%d] = %s\n", ii, arg->args[ii]);
            ii++;
        }
    }
    arg->numArgs = ii;
}

char *findPath(arguments *arg) {
    char *path = getenv("PATH");
    char *p = strdup(path);
    char *token = NULL;
    char *delim = ":";
    struct stat s;
    char *execPath = (char *) malloc(strlen(path));
//    arg[strlen(arg)] = '\0';

    fprintf(stdout, "PATH = %s\n", p);
    token = strtok(p, delim);
    while (token != NULL) {
        fprintf(stdout, "token = %s\n", token);
        strcpy(execPath, token);
        strcat(execPath, "/");
        fprintf(stdout, "execPath = %s\n", execPath);
        strcat(execPath, arg->args[0]);
        fprintf(stdout, "execPath = %s\n", execPath);
        if (stat(execPath, &s) != -1) {
            fprintf(stdout, "%s is in %s\n",arg->args[0], execPath);
            arg->path = strdup(execPath);
            return execPath;
        }
        else {
            fprintf(stdout, "stat == -1\n");
        }
//        fprintf(stdout, "before memset\n");
        memset(execPath, 0, sizeof(*execPath));
//        fprintf(stdout, "after memset\n");
        token = strtok(NULL, delim);
    }
    fprintf(stdout, "GOT HERE\n");
//    memset(execPath, 0, sizeof(*execPath));
//    fprintf(stdout, "after memset\n");
//    cwd = getcwd(NULL, 0);
//    fprintf(stdout, "cwd = %s\n", cwd);
//    strlcat(cwd, arg, sizeof(cwd));
//    execPath = "./";
//    fprintf(stdout, "execPath = %s\n", execPath);
//    strlcat(execPath, arg, )
    if (stat(arg->args[0], &s) != -1) {
        fprintf(stdout, "%s is in %s\n",arg->args[0], execPath);
        arg->path = strdup(execPath);
        return execPath;
    }
    fprintf(stdout, "ERROR: %s not found!\n", arg->args[0]);

    return NULL;
}

int main(int argc, char *argv[]) {
   char *path = NULL;
   char *line = NULL;
   size_t len = 0;
   char *cwd;
   arguments *args = (arguments *) malloc(sizeof(arguments));
   args->args = (char **) malloc(sizeof(char *));
   args->noWait = 0;

    if (argc == 1) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
        while (1) {
            cwd = getcwd(NULL, 0);
            fprintf(stdout, "%s> ", cwd);
            getline(&line, &len, stdin);
            getArgs(line, args);
            if ((path = findPath(args)) != NULL) {
                fprintf(stdout, "got here\n");
                fprintf(stdout, "path = %s\n", path);
                execCMD(*args);
            }
        }
#pragma clang diagnostic pop
    }
    return 0;
}