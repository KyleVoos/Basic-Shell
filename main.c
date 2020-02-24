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
} usrIn;

char *removeWS(char *str) {
    while( isspace(*(++str)) ); // Trim leading space

    if( (*str) == '\0' ) {
        return str;
    }

    size_t len = strlen(str);
    len--;

    while ( isspace(*(str +len)) ) {
        len--;
    }

    *(str + len + 1) = '\0';

    return (str-1);
}

// function to free the memory in the struct
void freeMem(usrIn *arg) {

    // loop through the array and clear everything in it
    for (int ii = 0; ii < arg->numArgs; ii++) {
        free(arg->args[ii]);
        arg->args[ii] = NULL;
    }
    arg->numArgs = 0;
    arg->noWait = 0;
}

// function to implement cd, should be self explanatory
void changeDir(usrIn arg) {

    if (arg.numArgs == 1)
        arg.args[1] = strdup("~/home");
    else if (arg.numArgs > 2) {
        fprintf(stderr, "-esh: cd: too many arguments\n");
        return;
    }
    if (chdir(arg.args[1]) == -1)
        fprintf(stderr, "%s\n", strerror(errno));

}

// function that execs the cmd from user input
void execCMD(usrIn arg) {
    pid_t pid;
    int wstatus;

    // fork a new child process
    if ((pid = fork()) == -1) {
        fprintf(stderr, "%s\n", strerror(errno));
        return;
    }

    // the child process that will exec the cmd
    if (pid == 0) {
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
void getArgs(char *input, usrIn *arg) {
    char *token;
    char *delim = " ";
    int arg_indx = 1;

    // use strtok to split the input string on ' '
    token = strtok(input, delim);

    if (token[strlen(token) - 1] == '\n') token[strcspn(token, "\n")] = '\0';

    // dup the first token into the first position of the array of arguments
    arg->args[0] = strdup(token);

    // continue looping through the rest of the user input
    while ((token = strtok(NULL, delim)) != NULL) {

        // remove the newline character from the token if one is found
        if (token[strlen(token) - 1] == '\n') token[strcspn(token, "\n")] = '\0';
        if (strcmp(token, "&") == 0) arg->noWait = 1; // set the noWait flag if & is detected
        else { // if & isn't detected dup the token into the array
            arg->args[arg_indx] = strdup(token);
            arg_indx++;
        }
    }
    arg->numArgs = arg_indx;  // finally set the number of arguemnts to ii, used for clearing memory
    arg->args[arg_indx] = NULL;
}

// attempts to find the path from the PATH environment variable
char *findPath(usrIn *arg) {
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
    token = strtok(path, delim);
    size_t len = strlen(token);

    // loop through all of the tokens in path to attempt to find the executable for arg->arg[0]
    while (token != NULL) {

        // dynamically reallocate memory for the execPath which will be the combination of the token and temp
        execPath = (char *) realloc(execPath, (sizeof(char) * (len + tempLen + 1)));
        strncpy(execPath, token, (len + tempLen));
        strncat(execPath, temp, (len + tempLen));
        execPath[strlen(execPath)] = '\0';

        // use stat on exec path, it will return 0 if this is the correct path to the executable in arg->args[0]
        if (stat(execPath, &s) != -1) {
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

    // check if arg->arg[0] is in the CWD, if not print the error message and return to main to get user input again
    if (stat(cwd, &s) != -1) {
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
   char *user_input = NULL;
   size_t len = 0;

   // variable for the struct and allocate memory for it
   usrIn *args = (usrIn *) malloc(sizeof(usrIn));

   // allocate memory for the array that will hold the arguments
   args->args = (char **) malloc(6 * sizeof(char *));

    if (argc == 1) {
        while (1) {
            // set the no wait flag to 0
            args->noWait = 0;
            fprintf(stdout, "\nehs> ");
            getline(&user_input, &len, stdin);
            char *parsed_input = removeWS(user_input);
            if (strlen(parsed_input) > 0) {
                // after getting the user input get the arguments from it
                getArgs(parsed_input, args);
                // check if the first argument is 'exit'
                if (strcmp(args->args[0], "exit") == 0) {
                    // free all the memory and terminate execution
                    free(user_input);
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
                    // exec the command and free the memory
                    execCMD(*args);
                    freeMem(args);
                    free(args->path);
                }
            }
            if (strlen(user_input) > 1) {
                free(user_input);
                len = 0;
            }
        }
    }
    return 0;
}
