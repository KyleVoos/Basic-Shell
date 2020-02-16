#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>
#include <bsd/string.h>

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
//    free(arg->args);
//    arg->args = NULL;
//    free(arg->path);
//    arg->path = NULL;
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
        token[strcspn(token, "\n")] = '\0';
//    fprintf(stdout, "token = %s\n", token);
    arg->args[0] = strdup(token);
    fprintf(stdout, "args[0] = %s\n", arg->args[0]);
    while ((token = strtok(NULL, delim)) != NULL) {
        if (token[strlen(token) - 1] == '\n')
            token[strcspn(token, "\n")] = '\0';
//        fprintf(stdout, "token = %s\n", token);
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
    char *execPath = (char *) malloc(sizeof(char));
    char temp[strlen(arg->args[0]) + 2];
    size_t len = 0;

    memset(temp, 0, sizeof(temp));

    strncpy(temp, "/", strlen(arg->args[0]) + 1);
    strncat(temp, arg->args[0], strlen(arg->args[0]) + 1);
    temp[strlen(temp)] = '\0';
    fprintf(stdout, "temp = %s\n", temp);
    fprintf(stdout, "PATH = %s\n", p);
    token = strtok(p, delim);
    len = strlen(token);
    while (token != NULL) {
        execPath = (char *) realloc(execPath, (sizeof(char) * (len + strlen(temp) + 1)));
//        fprintf(stdout, "token = %s\n", token);
//        fprintf(stdout, "exec length = %d\n", (int) (strlen(token) + strlen(temp)));
        strncpy(execPath, token, (len + strlen(temp)));
//        fprintf(stdout, "execPath = %s\n", execPath);
        strncat(execPath, temp, (len + strlen(temp)));
        execPath[strlen(execPath)] = '\0';
        fprintf(stdout, "exec = %s\n", execPath);
        if (stat(execPath, &s) != -1) {
            fprintf(stdout, "%s is in %s\n",arg->args[0], execPath);
            arg->path = strdup(execPath);
            free(p);
            free(execPath);
            return arg->path;
        }
        token = strtok(NULL, delim);
        if (token != NULL) len = strlen(token);
        memset(execPath, 0, sizeof(*execPath));
        fprintf(stdout, "end of while loop\n");
    }
    fprintf(stdout, "GOT HERE\n");

    char cwd[strlen(arg->args[0]) + 3];
    strncpy(cwd, "./", strlen(arg->args[0]) + 2);
    strncat(cwd, arg->args[0], strlen(arg->args[0]) + 2);
    cwd[strlen(cwd)] = '\0';
    fprintf(stdout, "cwd = %s\n", cwd);
    if (stat(cwd, &s) != -1) {
        fprintf(stdout, "%s is in %s\n",arg->args[0], getcwd(NULL, 0));
        arg->path = strdup(cwd);
        return execPath;
    }
    free(execPath);
    free(p);
    fprintf(stdout, "ERROR: %s not found!\n", arg->args[0]);

    return NULL;
}

int main(int argc, char *argv[]) {
   char *line = NULL;
   size_t len = 0;
   char *cwd;
   arguments *args = (arguments *) malloc(sizeof(arguments));
   args->args = (char **) malloc(6 * sizeof(char *));

    if (argc == 1) {
        while (1) {
            args->noWait = 0;
//            cwd = getcwd(NULL, 0);
            fprintf(stdout, "ehs> ");
            getline(&line, &len, stdin);
            if (strcmp(&line[0], "\n") != 0) {
                fprintf(stdout, "GOT HERE\n");
                getArgs(line, args);
                if (strcmp(args->args[0], "exit") == 0) {
                    free(line);
//                    free(cwd);
                    freeMem(args);
                    free(args->args);
                    args->args = NULL;
                    free(args);
                    return 0;
                }
                if (strcmp(args->args[0], "cd") == 0) {
                    changeDir(*args);
                    freeMem(args);
                } else if ((args->path = findPath(args)) != NULL) {
                    fprintf(stdout, "args->path = %s\n", args->path);
                    execCMD(*args);
                    freeMem(args);
                    free(args->path);
                }
            }
            free(line);
//            free(cwd);
        }
    }
    return 0;
}