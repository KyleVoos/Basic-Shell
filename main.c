#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>

void execCMD(char *path, char **args) {
    pid_t pid;
    int wstatus;

    if ((pid = fork()) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        return;
    }

    if (pid == 0) {
        execv(path, args);
    }
    else {
        wait(&wstatus);
    }
}

void getArgs(char *input, char **args) {
    char *token;
    char *delim = " ";
//    char **args = (char **) malloc(sizeof(char *));
    int ii = 1;

    token = strtok(input, delim);
    if (token[strlen(token) - 1] == '\n')
        token[strlen(token) - 1] = '\0';
    fprintf(stdout, "token = %s\n", token);
    args[0] = strdup(token);
    fprintf(stdout, "args[0] = %s\n", args[0]);
    while ((token = strtok(NULL, delim)) != NULL) {
        if (token[strlen(token) - 1] == '\n')
            token[strlen(token) - 1] = '\0';
        fprintf(stdout, "token = %s\n", token);
        args[ii] = strdup(token);
        fprintf(stdout, "args[%d] = %s\n",ii, args[ii]);
        ii++;
    }
}

char *findPath(char *arg) {
    char *path = getenv("PATH");
    char *p = strdup(path);
    char *token;
    char *delim = ":";
    struct stat s;
    char *execPath;
    arg[strlen(arg)] = '\0';

    fprintf(stdout, "PATH = %s\n", p);
    token = strtok(p, delim);
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
        if ((token = strtok(NULL, delim)) == NULL) {
            break;
        }
        memset(execPath, 0, sizeof(*execPath));
    }
    fprintf(stdout, "GOT HERE");
    memset(execPath, 0, sizeof(execPath));
    execPath = getcwd(NULL, 0);
    strlcat(execPath, arg, sizeof(execPath));
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
   char *cwd;

    if (argc == 1) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
        while (1) {
            cwd = getcwd(NULL, 0);
            fprintf(stdout, "%s>", cwd);
            getline(&line, &len, stdin);
            getArgs(line, args);
            if ((path = findPath(args[0])) != NULL) {
                fprintf(stdout, "got here\n");
                fprintf(stdout, "path = %s\n", path);
                execCMD(path, args);
            }
        }
#pragma clang diagnostic pop
    }
    return 0;
}