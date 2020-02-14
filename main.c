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

void freeMem(arguments *arg) {

    for (int ii = 0; ii < arg->numArgs; ii++) {
        free(arg->args[ii]);
        arg->args[ii] = NULL;
    }
    free(arg->args);
    arg->args = NULL;
    free(arg->path);
    arg->path = NULL;
    arg->numArgs = 0;
    arg->noWait = 0;
}

void changeDir(arguments arg) {

    if (arg.numArgs == 1)
        arg.args[1] = strdup("~/home");
    else if (arg.numArgs > 2) {
        fprintf(stdout, "-esh: cd: too many arguments\n");
        return;
    }
    if (chdir(arg.args[1]) == -1)
        fprintf(stderr, "%s\n", strerror(errno));

}

void execCMD(arguments arg) {
    pid_t pid;
    int wstatus;

    fprintf(stdout, "number of arguments = %d\n", arg.numArgs);

    if ((pid = fork()) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        return;
    }

    if (pid == 0) {
        if (arg.noWait == 1) sleep(1);
        execv(arg.path, arg.args);
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
    else if (arg.noWait == 0) {
        wait(&wstatus);
        return;
//        if (arg.noWait == 0)
//            wait(&wstatus);
//        else {
//            fprintf(stdout, "& detected\n");
//            return;
//        }
    }
}

void getArgs(char *input, arguments *arg) {
    char *token;
    char *delim = " ";
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
    char *execPath = NULL;

    fprintf(stdout, "PATH = %s\n", p);
    token = strtok(p, delim);
    while (token != NULL) {
        execPath = (char *) malloc(strlen(path));
//        fprintf(stdout, "token = %s\n", token);
        strlcpy(execPath, token, sizeof(execPath));
        strcat(execPath, "/");
//        fprintf(stdout, "execPath = %s\n", execPath);
        strcat(execPath, arg->args[0]);
//        fprintf(stdout, "execPath = %s\n", execPath);
        if (stat(execPath, &s) != -1) {
            fprintf(stdout, "%s is in %s\n",arg->args[0], execPath);
            arg->path = strdup(execPath);
            free(p);
            free(execPath);
            return execPath;
        }
//        else {
//            fprintf(stdout, "stat == -1\n");
//        }
//        memset(execPath, 0, sizeof(*execPath));
        free(execPath);
        token = strtok(NULL, delim);
    }
//    fprintf(stdout, "GOT HERE\n");
//    memset(execPath, 0, sizeof(*execPath));
//    fprintf(stdout, "after memset\n");
//    cwd = getcwd(NULL, 0);
//    fprintf(stdout, "cwd = %s\n", cwd);
//    strlcat(cwd, arg, sizeof(cwd));
//      execPath = "./";
//      fprintf(stdout, "execPath = %s\n", execPath);
//      strlcat(execPath, arg->args[0], sizeof(execPath));
    if (stat(arg->args[0], &s) != -1) {
        fprintf(stdout, "%s is in %s\n",arg->args[0], getcwd(NULL, 0));
        arg->path = strdup(execPath);
        return execPath;
    }
//    free(execPath);
    free(p);
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

    if (argc == 1) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
        while (1) {
            args->noWait = 0;
            cwd = getcwd(NULL, 0);
            fprintf(stdout, "ehs: %s> ", cwd);
            getline(&line, &len, stdin);
            getArgs(line, args);
            if (strcmp(args->args[0], "exit") == 0) {
                free(line);
                free(cwd);
                freeMem(args);
                free(args);
                return 0;
            }
            if (strcmp(args->args[0], "cd") == 0) {
                changeDir(*args);
            }
            else if ((path = findPath(args)) != NULL) {
//                fprintf(stdout, "got here\n");
                fprintf(stdout, "path = %s\n", path);
                execCMD(*args);
            }
            free(line);
            free(cwd);
        }
#pragma clang diagnostic pop
    }
    return 0;
}