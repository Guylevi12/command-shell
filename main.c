#include <stdio.h>
#include <string.h>
#include <unistd.h>
#include <sys/wait.h>
#include <stdlib.h>
#include <stdbool.h>

#define MAX_INPUT_LENGTH 1024//max input length including the null terminator
#define MAX_ARGS 6//max number of arguments (command +4 arguments)


typedef struct Alias {
    char name[MAX_INPUT_LENGTH];
    char command[MAX_INPUT_LENGTH];
    struct Alias *next;
} Alias;

typedef struct Jobs {
    int index;
    pid_t id;
    char name[MAX_INPUT_LENGTH];
    struct Jobs *next;
} Jobs;


//global variables for certain elements like counters and checks
Jobs *jobsList = NULL;
Alias *head = NULL;
int cmdCount = 0;
int aliasCount = 0;
int scriptLineCount = 0;
int apostropheCount = 0;
FILE *file = NULL;
bool error_flag = false;
bool orAndFlag = false;
bool is_apostrophe = false;
bool isJobs = false;
bool is2 = false;
bool isInBrackets = false;


//running the terminal finding an argument transforming the input to output,free args and trim the spaces
void runShell(char *args[]);

char *find_argument(char **input);

void input_to_argument(char *input);

void free_arg(char *args[]);

char *trimSpaces(const char *str);

//functions to read a script file and function to handle sigchld
//void open_new_error_file(char *filename);

void error_handler_to_file(char *args[], char *input, int index);

void read_Script(char *name, char *args[]);

void sigchld_handler(int signum);

//functions to handle adding jobs and deleting them
void addJob(pid_t id, char *args[]);

void deleteJob(pid_t id);

void freeJobs();

void printJobs();

void printReverse(Jobs *job, int *index);


//functions for the alias
Alias *createNode(const char *name, const char *command);

void addAlias(char *name, char *command);

void removeAlias(char *name);

void printAlias();

void freeAlias();


void sigchld_handler(int signum) {
    int status;
    pid_t pid;

    while ((pid = waitpid(-1, &status, WNOHANG)) > 0) {
        if (WIFEXITED(status) && WEXITSTATUS(status) == 0) {
            cmdCount++;
            if (is_apostrophe) {
                apostropheCount++;
            }
        } else {
            error_flag = true;
        }
        deleteJob(pid);
    }
}

int main() {
    struct sigaction sa;
    sa.sa_handler = sigchld_handler;
    sigemptyset(&sa.sa_mask);
    sa.sa_flags = SA_RESTART;

    if (sigaction(SIGCHLD, &sa, NULL) == -1) {
        perror("sigaction");
        exit(1);
    }

    char input[MAX_INPUT_LENGTH];
    while (1) {
        //print prompt
        printf("#cmd:%d|#alias:%d|#script lines:%d> ", cmdCount, aliasCount, scriptLineCount);//initial prompt start
        fflush(stdout);
        if (fgets(input, MAX_INPUT_LENGTH, stdin) == NULL) {//get an input from the user
            perror("fgets");
            exit(1);
        }
        if (strlen(input) == 1) {//if the input is a space bar continue
            continue;
        }
        input[strcspn(input, "\n")] = '\0';
        //exit shell if input is "exit_shell"
        char *temp = trimSpaces(input);
        if (strcmp(temp, "exit_shell") == 0) {//exit command to exit the program
            free(temp);
            temp = NULL;
            printf("%d\n", apostropheCount);
            break;
        }
        free(temp);
        temp = NULL;
        is2 = false;
        input_to_argument(input);
    }
    freeAlias();
    freeJobs();
    return 0;
}

void addJob(pid_t id, char *args[]) {
    Jobs *newJob = (Jobs *) malloc(sizeof(Jobs));
    if (newJob == NULL) {
        perror("malloc");
        exit(1);
    }
    newJob->id = id;
    unsigned long len = 0;
    for (int i = 0; args[i] != NULL; ++i) {
        len += strlen(args[i]);
        if (i > 0) {
            len++;
        }
    }
    if (len >= MAX_INPUT_LENGTH) {
        error_flag = true;
        free(newJob);
        newJob = NULL;
        return;
    }
    newJob->name[0] = '\0';
    for (int i = 0; args[i] != NULL; ++i) {
        strcat(newJob->name, args[i]);
        if (args[i + 1] != NULL) {
            strcat(newJob->name, " ");
        }
    }
    newJob->next = jobsList;
    jobsList = newJob;
}

void deleteJob(pid_t id) {
    Jobs *current = jobsList;
    Jobs *previous = NULL;

    while (current != NULL && current->id != id) {
        previous = current;
        current = current->next;
    }
    if (current == NULL) {
        return;
    }
    if (previous == NULL) {
        jobsList = current->next;
    } else {
        previous->next = current->next;
    }
    if (strcmp(current->name, "sleep") != 0) {
        return;
    }
    cmdCount++;
    free(current);
    current = NULL;
}

void freeJobs() {
    Jobs *current = jobsList;
    while (current != NULL) {
        Jobs *next = current->next;
        free(current);
        current = next;
    }
    jobsList = NULL;
}

void printReverse(Jobs *job, int *index) {
    if (job == NULL) {
        return;
    }
    printReverse(job->next, index);
    printf("[%d]               %s &\n", (*index)++, job->name);
}

void printJobs() {
    Jobs *current = jobsList;
    if (jobsList == NULL) {
        return;
    }
    current->index = 1;
    printReverse(current, &current->index);
}


Alias *createNode(const char *name, const char *command) {//create a linked List of containing the aliases
    Alias *newNode = (Alias *) malloc(sizeof(Alias));
    if (newNode == NULL) {
        error_flag = true;
        exit(1);
    }
    strncpy(newNode->name, name, MAX_INPUT_LENGTH);
    strncpy(newNode->command, command, MAX_INPUT_LENGTH);
    newNode->next = NULL;
    return newNode;
}

void addAlias(char *name, char *command) {//add a new alias to the list
    Alias *current = head;
    while (current != NULL) {
        if (strcmp(current->name, name) == 0) {
            strncpy(current->command, command, MAX_INPUT_LENGTH - 1);
            current->command[MAX_INPUT_LENGTH - 1] = '\0';
            return;
        }
        current = current->next;
    }
    Alias *newNode = createNode(name, command);
    newNode->next = head;
    head = newNode;
    aliasCount++;
}

void removeAlias(char *name) {//remove an alias from the list
    Alias *current = head;
    Alias *previous = NULL;
    while (current != NULL && strcmp(current->name, name) != 0) {
        previous = current;
        current = current->next;
    }
    if (current == NULL) {
        error_flag = true;
        return;
    }
    if (previous == NULL) {
        head = current->next;
    } else {
        previous->next = current->next;
    }
    free(current);
    current = NULL;
    aliasCount--;
}

void printAlias() {//print the list of aliases
    if (aliasCount == 0) {
        error_flag = true;
        return;
    }
    Alias *current = head;
    while (current != NULL) {
        printf("%s='%s'\n", current->name, current->command);
        current = current->next;
    }
}

void freeAlias() {//free the alias
    Alias *current = head;
    while (current != NULL) {
        Alias *next = current->next;
        free(current);
        current = next;
    }
    head = NULL;
}

char *trimSpaces(const char *str) {//trim the spaces between and tabs in the word
    char *output = strdup(str);
    int i = 0, j = 0;
    bool inSpaces = false;
    while (str[i] == ' ' || str[i] == '\t') {
        i++;
    }
    while (str[i] != '\0') {
        if (str[i] == ' ' || str[i] == '\t') {
            if (!inSpaces) {
                output[j++] = ' ';
                inSpaces = true;
            }
        } else {
            output[j++] = str[i];
            inSpaces = false;
        }
        i++;
    }
    if (j > 0 && output[j - 1] == ' ') {
        j--;
    }
    output[j] = '\0';
    return output;
}

char *find_argument(char **input) {
    if (input == NULL) {
        return NULL;
    }
    while (**input == ' ' || **input == '\t') {
        (*input)++;
    }
    if (**input == '\0') {
        return NULL;
    }
    char *res = (char *) malloc(MAX_INPUT_LENGTH);
    if (res == NULL) {
        perror("malloc");
        exit(1);
    }
    int i = 0;
    if (**input == '\'' || **input == '"' || **input == '(') {
        char stop;
        if (**input == '(') {
            stop = ')';
        } else {
            stop = **input;
        }
        (*input)++;
        while (**input != stop) {
            if (**input == '\0') {
                error_flag = true;
                free(res);
                return NULL;
            }
            res[i] = **input;
            i++;
            (*input)++;
        }
        if (stop == ')') {
            isInBrackets = true;
        } else {
            is_apostrophe = true;
        }
        (*input)++;
        res[i] = '\0';
        return res;
    }
    while (**input != '\0' && **input != '\n' && **input != '\'' && **input != '"' && **input != ' ' &&
           **input != ')') {
        res[i] = **input;
        i++;
        (*input)++;
    }
    res[i] = '\0';
    return res;
}

void error_handler_to_file(char *args[], char *input, int index) {
    char *error_filename = NULL;
    args[index] = find_argument(&input);
    index--;
    if (args[index] == NULL || index >= MAX_ARGS - 1) {
        error_flag = true;
    }
    error_filename = strdup(args[index]);
    if (strcmp(error_filename, "2>") == 0) {
        error_flag = true;
        return;
    }
    free(args[index]);
    args[index] = NULL;
    free(args[--index]);
    args[index] = NULL;
    file = fopen(error_filename, "w");
    if (file == NULL) {
        perror("fopen");
        error_flag = true;
        return;
    }
    free(error_filename);
    error_filename = NULL;
    if (error_flag) {
        fprintf(file, "ERR\n");
        fclose(file);
        file = NULL;
        return;
    }
    if (isInBrackets) {
        input_to_argument(args[0]);
    } else {
        runShell(args);
    }
    fclose(file);
    file = NULL;
}

void input_to_argument(char *input) {
    isInBrackets = false;
    is_apostrophe = false;
    error_flag = false;
    orAndFlag = false;
    isJobs = false;
    char *args[MAX_ARGS];
    for (int i = 0; i < MAX_ARGS; ++i) {
        args[i] = NULL;
    }
    int index = 0;
    char *temp = find_argument(&input);
    if (temp == NULL) {
        if (error_flag) {
            printf("ERR\n");
        }
        return;
    }
    args[index] = strdup(temp);
    free(temp);
    Alias *tempAlias = head;
    while (tempAlias != NULL) {
        if (strcmp(args[index], tempAlias->name) == 0) {
            free_arg(args);
            char *alias_command = strdup(tempAlias->command);
            while (index < MAX_ARGS && !error_flag) {
                temp = find_argument(&alias_command);
                if (temp == NULL) {
                    break;
                }
                args[index] = strdup(temp);
                free(temp);
                if (strcmp(args[index], "&&") == 0 || strcmp(args[index], "||") == 0 || strcmp(args[index], "&") == 0 ||
                    strcmp(args[index], "2>") == 0) {
                    if (strcmp(args[index], "&") == 0) {
                        isJobs = true;
                        free(args[index]);
                        args[index] = NULL;
                        break;
                    } else if (strcmp(args[index], "2>") == 0) {
                        is2 = true;
                    } else {
                        orAndFlag = true;
                        break;
                    }
                }
                index++;
            }
            index--;
            break;
        }
        tempAlias = tempAlias->next;
    }
    index++;
    while (index < MAX_ARGS - 1 && !error_flag) {
        temp = find_argument(&input);
        if (temp == NULL) {
            break;
        }
        args[index] = strdup(temp);
        free(temp);
        if (strcmp(args[index], "&&") == 0 || strcmp(args[index], "||") == 0) {
            orAndFlag = true;
            break;
        } else if (strcmp(args[index], "&") == 0) {
            isJobs = true;
            free(args[index]);
            args[index] = NULL;
        } else if (strcmp(args[index], "2>") == 0) {
            is2 = true;
        }
        index++;
    }
    if (!orAndFlag) {
        temp = find_argument(&input);
        if (temp != NULL) {
            args[index] = strdup(temp);
            free(temp);
            if (strcmp(args[index], "&&") == 0 || strcmp(args[index], "||") == 0) {
                orAndFlag = true;
            } else if (strcmp(args[index], "&") == 0) {
                isJobs = true;
                free(args[index]);
                args[index] = NULL;
                if (index == MAX_ARGS - 1) {
                    error_flag = true;
                }
            } else if (strcmp(args[index], "2>") == 0) {
                is2 = true;
            }
            index++;
        }
    }
    if (!orAndFlag && args[MAX_ARGS - 1] != NULL) {
        error_flag = true;
        if (!is2 || (is2 && file != NULL)) {
            index = 0;
            while (!orAndFlag) {
                free_arg(args);
                temp = find_argument(&input);
                if (temp == NULL) {
                    break;
                }
                args[index] = strdup(temp);
                free(temp);
                if (strcmp(args[index], "&&") == 0 || strcmp(args[index], "||") == 0) {
                    orAndFlag = true;
                } else if (strcmp(args[index], "2>") == 0) {
                    is2 = true;
                    index++;
                    temp = find_argument(&input);
                    if (temp == NULL) {
                        error_flag = true;
                        break;
                    }
                    args[index] = strdup(temp);
                    free(temp);
                    index++;
                    break;
                }
            }
        }
    }
    if (is2 && file == NULL) {
        return error_handler_to_file(args, input, index);
    }
    if (error_flag) {
        if (is2) {
            fprintf(file, "ERR\n");
        } else {
            printf("ERR\n");
        }
    }
    char multipleCommand[3];
    multipleCommand[0] = '\0';
    if (orAndFlag) {
        strcpy(multipleCommand, args[index]);
        free(args[index]);
        args[index] = NULL;
        if (!error_flag) {
            runShell(args);
            free_arg(args);
        }
        if ((strcmp(multipleCommand, "&&") == 0 && error_flag)) {
            args[index] = find_argument(&input);
            while (args[index] != NULL) {
                if ((strcmp(args[index], "||") == 0)) {
                    free(args[index]);
                    args[index] = NULL;
                    return input_to_argument(input);
                }
                free(args[index]);
                args[index] = NULL;
                args[index] = find_argument(&input);
            }
        } else if (((strcmp(multipleCommand, "||") == 0) && !error_flag)) {
            free_arg(args);
            return;
        }
        return input_to_argument(input);
    } else {
        if (error_flag) {
            free_arg(args);

            return;
        }
        runShell(args);
        free_arg(args);
    }
}

void free_arg(char *args[]) {//free the argument
    for (int i = 0; i < MAX_ARGS; i++) {
        if (args[i] != NULL) {
            free(args[i]);
            args[i] = NULL;
        }
    }
}

void read_Script(char *name, char *args[]) {//read a script and run through it using the shell.
    free_arg(args);
    FILE *fp;
    int n = (int) strlen(name);
    if (n <= 3 || name[n - 1] != 'h' || name[n - 2] != 's' || name[n - 3] != '.') {
        error_flag = true;
        free(name);
        name = NULL;
        return;
    }
    fp = fopen(name, "r");
    free(name);
    name = NULL;
    if (fp == NULL) {
        error_flag = true;
        perror("Error opening file");
        return;
    }
    char command[MAX_INPUT_LENGTH];
    if (fgets(command, MAX_INPUT_LENGTH, fp) == NULL || strcmp(command, "#!/bin/bash\n") != 0) {
        error_flag = true;
        fclose(fp);
        free_arg(args);
        free(name);
        name = NULL;
        return;
    }
    while ((fgets(command, MAX_INPUT_LENGTH, fp)) != NULL) {
        scriptLineCount++;
        if (strlen(command) == 1 || command[0] == '#') {
            continue;
        }
        command[strcspn(command, "\n")] = '\0';
        input_to_argument(command);
    }
    fclose(fp);
}

void runShell(char *args[]) {//run the shell
    int status;
    if (strcmp(args[0], "jobs") == 0) {
        if (args[1] != NULL) {
            error_flag = true;
            printf("ERR\n");
            return;
        }
        printJobs();
        cmdCount++;
        return;
    }
    if (strcmp(args[0], "unalias") == 0) {
        if (args[1] == NULL) {
            error_flag = true;
            return;
        }
        char *unaliasName = strdup(args[1]);
        if (unaliasName == NULL) {
            error_flag = true;
            return;
        }
        removeAlias(unaliasName);
        free(unaliasName);
        if (is_apostrophe)
            apostropheCount++;
        cmdCount++;
        return;
    } else if (strcmp(args[0], "alias") == 0) {
        if (args[1] == NULL) {
            printAlias();
            if (is_apostrophe)
                apostropheCount++;
            cmdCount++;
            return;
        }
        if (args[2] == NULL) {
            error_flag = true;
            return;
        }
        if (strcmp(args[2], "=") == 0) {
            if (args[3] == NULL) {
                error_flag = true;
                return;
            }
            addAlias(args[1], args[3]);
            return;
        }
        if (args[1][strlen(args[1]) - 1] != '=') {
            error_flag = true;
            return;
        }
        args[1][strlen(args[1]) - 1] = '\0';
        addAlias(args[1], args[2]);
        if (is_apostrophe)
            apostropheCount++;
        cmdCount++;
        return;
    }

    if (strcmp(args[0], "source") == 0) {
        if (args[1] == NULL || args[2] != NULL) {
            printf("ERR\n");
            return;
        }
        char *file_name = strdup(args[1]);
        free_arg(args);
        read_Script(file_name, args);
        free_arg(args);
        return;
    }
    //fork a child process
    pid_t pid = fork();
    if (pid < 0) {
        perror("fork");
        free_arg(args);
        exit(1);
    } else if (pid == 0) {//child process
        if (is2) {
            cmdCount++;
            int fp = fileno(file);
            if (dup2(fp, STDERR_FILENO) == -1) {
                perror("dup2");
                exit(1);
            }
        }
        //execute the command
        execvp(args[0], args);
        perror("execvp");
        if (file != NULL) {
            fclose(file);
            file = NULL;
        }
        free_arg(args);
        freeJobs();
        _exit(1);
    } else {//parent process
        if (isJobs) {
            addJob(pid, &args[0]);
//            cmdCount++;
        } else {
            waitpid(pid, &status, 0);
            if (status == 0) {
                if (is_apostrophe)
                    apostropheCount++;
                cmdCount++;
            } else {
                error_flag = true;
            }
        }
    }
}

