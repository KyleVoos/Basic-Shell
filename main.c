#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <bsd/string.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>

typedef struct _arguments {
    char *path;
    char **args;
    int numArgs;
    int noWait; // For '&' flag
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
    int argsIndx = 1;
    int maxArgs = 5;

    token = strtok(input, delim);
    int lastTokIndx = strlen(token) - 1;
    token[lastTokIndx] = (token[lastTokIndx] == '\n')? '\0': token[lastTokIndx];
    arg->args[0] = strdup(token);
    fprintf(stdout, "args[0] (token) = %s\n", arg->args[0]);

    // Needs to check if there are 5

    while ((token = strtok(NULL, delim)) != NULL) {
        if (maxArgs == 0) {// we're only accepting 5 args
            fprintf(stderr,"Only accepting 5 arguments.\n");
            break;
        }

        lastTokIndx = strlen(token) - 1;
        token[lastTokIndx] = (token[lastTokIndx] == '\n')? '\0': token[lastTokIndx];
        
        fprintf(stdout, "token = %s\n", token);
        if (strcmp(token, "&") == 0) {
            arg->noWait = 1;
        }
        else {
            arg->args[argsIndx] = strdup(token);
            fprintf(stdout, "args[%d] = %s\n", argsIndx, arg->args[argsIndx]);
            argsIndx++;
        }
        maxArgs--;
    }
    arg->numArgs = argsIndx;
}

char *findPath(arguments *arg) {
    char *path = getenv("PATH");
    int pathLen = strlen(path);
    char *token = NULL;
    char delim[] = ":";
    char *execPath = NULL;
    struct stat s;

    // Just check the command and not reference it to path yet
    if (stat(arg->args[0], &s) != -1) {
        fprintf(stdout, "%s is in %s\n",arg->args[0], getcwd(NULL, 0));
        arg->path = strdup(execPath);
        return execPath;
    }

    // Concat our cmd to each token of path until we find
    // the location of the cmd (if there is one)
    token = strtok(path, delim);
    while (token != NULL) {
        execPath = (char *) malloc(pathLen *sizeof(char));
        strlcpy(execPath, token, sizeof(execPath));
        strcat(execPath, "/");
    //    fprintf(stdout, "execPath = %s\n", execPath);
        strcat(execPath, arg->args[0]);
    //    fprintf(stdout, "execPath = %s\n", execPath);

        // If we find the location of the actual command
        if (stat(execPath, &s) != -1) {
            fprintf(stdout, "%s is in %s\n",arg->args[0], execPath);
            arg->path = strdup(execPath);
            free(execPath);
            return arg->path;
        }
        free(execPath);
        token = strtok(NULL, delim);
    }

    fprintf(stdout, "ERROR: %s not found!\n", arg->args[0]);
    return NULL;
}

int main(int argc, char *argv[]) {
   char *path = NULL;
   char *line = NULL;
   size_t len = 0;
   char *cwd;
   arguments *args = (arguments *) malloc(sizeof(arguments)); // Making Head of LL
   args->args = (char **) malloc(5 *sizeof(char *)); // Set table to be size 5 (requirement)

    if (argc == 1) {
#pragma clang diagnostic push
#pragma clang diagnostic ignored "-Wmissing-noreturn"
        while (1) {
            args->noWait = 0;
            cwd = getcwd(NULL, 0);
            fprintf(stdout, "=============\nCWD: %s \n", cwd);
            fprintf(stdout, "esh> ");
            getline(&line, &len, stdin);
            getArgs(line, args);
            if (strcmp(args->args[0], "exit") == 0) {
                free(line);
                free(cwd);
                freeMem(args);
                free(args);
                return 0;
            }

            // Extra feature
            if (strcmp(args->args[0], "cd") == 0) {
                changeDir(*args);
            }
            else if ((path = findPath(args)) != NULL) {
//                fprintf(stdout, "got here\n");
                fprintf(stdout, "Got the command\n");
                execCMD(*args);
            }
            free(line);
            free(cwd);
        }
#pragma clang diagnostic pop
    }
    return 0;
}