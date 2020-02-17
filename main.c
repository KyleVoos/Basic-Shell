#include <stdio.h>
#include <stdlib.h>
#include <sys/stat.h>
#include <string.h>
#include <unistd.h>
#include <errno.h>
#include <wait.h>
#include <ctype.h>

// strcut to hold all of the variables we need
typedef struct _arguments {
    char *path; // this holds the path for what we are trying to exec
    char **args;    // array that holds the cmd in the first arg and all of the optional arguments
    int numArgs;    // the number of arguments
    int noWait;     // flag for if & was detected in input
} arguments;

// function to free the memory in the struct
void freeMem(arguments *arg) {

    // loop through the array and clear everything in it
    for (int ii = 0; ii < arg->numArgs; ii++) {
        free(arg->args[ii]);
        arg->args[ii] = NULL;
    }
    arg->numArgs = 0;
    arg->noWait = 0;
}

// function to implement cd, should be self explanatory
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

// function that execs the cmd from user input
void execCMD(arguments arg) {
    pid_t pid;
    int wstatus;

    fprintf(stdout, "number of arguments = %d\n", arg.numArgs);

    // fork a new child process
    if ((pid = fork()) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        return;
    }

    // the child process that will exec the cmd
    if (pid == 0) {
        // this was an attempt to test if the child process is execing in the background correctly
        if (arg.noWait == 1) sleep(1);
        // exec the commands, first arg is the path to the executable, second ard is the array with the executable and optional arguments
        execv(arg.path, arg.args);
        // gets here if exec fails
        fprintf(stderr, "%s\n", strerror(errno));
        exit(1);
    }
    // parent process, we only want to wait if & wasn't detected which means the flag will be 0
    else if (arg.noWait == 0) {
        // if & was detected we wait for the child process to finish then return back to main
        wait(&wstatus);
        return;
    }
}

// this fucntion gets the arguments from the user input and adds them to the struct
void getArgs(char *input, arguments *arg) {
    char *token;
    char *delim = " ";
    int ii = 1;

    // use strtok to split the input string on ' '
    token = strtok(input, delim);

    // probably need to remove newline character, this does if its detected
    if (token[strlen(token) - 1] == '\n')
        token[strcspn(token, "\n")] = '\0';
//    fprintf(stdout, "token = %s\n", token);

    // dup the first token into the first position of the array of arguments
    arg->args[0] = strdup(token);
    fprintf(stdout, "args[0] = %s\n", arg->args[0]);

    // continue looping through the rest of the user input
    while ((token = strtok(NULL, delim)) != NULL) {

        // remove the newline character from the token if one is found
        if (token[strlen(token) - 1] == '\n')
            token[strcspn(token, "\n")] = '\0';
//        fprintf(stdout, "token = %s\n", token);
        if (strcmp(token, "&") == 0) arg->noWait = 1; // set the noWait flag if & is detected
        else { // if & isn't detected dup the token into the array
            arg->args[ii] = strdup(token);
            fprintf(stdout, "args[%d] = %s\n", ii, arg->args[ii]);
            ii++;
        }
    }
    arg->numArgs = ii;  // finally set the number of arguemnts to ii, used for clearing memory
}

// attempts to find the path from the PATH environment variable
char *findPath(arguments *arg) {
    // get the PATH environment variable
    char *path = strdup(getenv("PATH"));
    char *token = NULL;
    char *delim = ":";
    struct stat s;
    // this holds the executable path, the memory for this is causing problems
    char *execPath = (char *) malloc(sizeof(char));
    size_t cmdLen = strlen(arg->args[0]);   // variable for size of arg->arg[0]
    char temp[cmdLen + 2];  // variable to hold '/' + arg->arg[0]

    // make the temp vairable that holds '/' + arg->arg[0]
    strncpy(temp, "/", (cmdLen + 1));
    strncat(temp, arg->args[0], (cmdLen + 1));
    temp[strlen(temp)] = '\0';  // null terminate temp
    size_t tempLen = strlen(temp);  // get the size of temp for the dynamic memory allocation for execPath

//    fprintf(stdout, "PATH = %s\n", path);

    token = strtok(path, delim);
    size_t len = strlen(token);

    // loop through all of the tokens in path to attempt to find the executable for arg->arg[0]
    while (token != NULL) {
        // dynamically reallocate memory for the execPath which will be the combination of the token and temp
        execPath = (char *) realloc(execPath, (sizeof(char) * (len + tempLen + 1)));
//        fprintf(stdout, "token = %s\n", token);
        strncpy(execPath, token, (len + tempLen));
        strncat(execPath, temp, (len + tempLen));
        execPath[strlen(execPath)] = '\0';
        fprintf(stdout, "execPath = %s\n", execPath);
        // use stat on exec path, it will return 0 if this is the correct path to the executable in arg->args[0]
        if (stat(execPath, &s) != -1) {
            fprintf(stdout, "%s is in %s\n",arg->args[0], execPath);
            arg->path = strdup(execPath);
            free(path);
            free(execPath);
            return arg->path;
        }
        // split the path variable again
        token = strtok(NULL, delim);
        // get the length of the new token, check if its NULL first to prevent segfault
        if (token != NULL) len = strlen(token);
    }

    // if the path for the executable was not found in PATH we need to look in the CWD
    char cwd[cmdLen + 3]; // the path for the cwd directory will be './' + arg->args[0] so the length will be 2 + strlen(arg->args[0]) + 1 for null terminate char
    strncpy(cwd, "./", cmdLen + 2); // build path for cwd
    strncat(cwd, arg->args[0], cmdLen + 2);
    cwd[strlen(cwd)] = '\0';
    fprintf(stdout, "cwd = %s\n", cwd);
    // check if arg->arg[0] is in the CWD, if not print the error message and return to main to get user input again
    if (stat(cwd, &s) != -1) {
        fprintf(stdout, "%s is in %s\n",arg->args[0], getcwd(NULL, 0));
        arg->path = strdup(cwd);
        free(execPath);
        free(path);
        return arg->args[0];
    }
    free(execPath);
    free(path);
    fprintf(stdout, "ERROR: %s not found!\n", arg->args[0]);

    return NULL;
}

int main(int argc, char *argv[]) {
   char *line = NULL;
   size_t len = 0;
   char *cwd;
   // variable for the struct and allocate memory for it
   arguments *args = (arguments *) malloc(sizeof(arguments));
   // allocate memory for the array that will hold the arguments
   args->args = (char **) malloc(6 * sizeof(char *));

    if (argc == 1) {
        while (1) {
            // set the no wait flag to 0
            args->noWait = 0;
            fprintf(stdout, "ehs> ");
            getline(&line, &len, stdin);
            if (strcmp(&line[0], "\n") != 0) {
                // after getting the user input get the arguments from it
                getArgs(line, args);
                // check if the first argument is 'exit'
                if (strcmp(args->args[0], "exit") == 0) {
                    // free all the memory and terminate execution
                    free(line);
                    freeMem(args);
                    free(args->args);
                    args->args = NULL;
                    free(args);
                    return 0;
                }
                // if the first argument is cd, take care of it
                if (strcmp(args->args[0], "cd") == 0) {
                    changeDir(*args);
                    freeMem(args);
                }
                // if the first argument isn't 'exit' or 'cd' we need to try to find the path to where the arguments executable is
                else if (findPath(args) != NULL) {   // if we were able to find the path, now exec it
                    fprintf(stdout, "args->path = %s\n", args->path);
                    // exec the command and free the memory
                    execCMD(*args);
                    freeMem(args);
                    free(args->path);
                }
            }
            if (strlen(line) > 1) {
                free(line);
                len = 0;
            }
        }
    }
    return 0;
}