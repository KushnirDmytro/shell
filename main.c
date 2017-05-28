// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include <regex.h>


static regex_t scriptRegex;
static regex_t assignRegex;
static int reti;

int shell_pwd(int argc, const char * argv[]);
int shell_cd(int argc, const char * argv[]);
int shell_exit(int argc, const char * argv[]);
static int help_flag = 0;
char *cwd_bufer;


struct pair {
    char * key;
    char * value;
};

typedef struct pair pair;

static pair * list_of_variables;
static int number_of_variables = 0;


int shell_pwd(int argc, const char * argv[]){
    static char help[] = "pwd [-h|--help] – вивести поточний шлях ";
    int wrong_options = 0;
    help_flag = 0;
    int option_index = 0;
    int opt;

    char *cp_ptr;

    static struct option long_options[] =
            {
                    {"help", no_argument, &help_flag, 1}
            };

    if (argc > 2) wrong_options=1;
    else {
        while (optind < argc) {
            if ((opt = getopt_long(argc, argv, "h::", long_options, &option_index)) != -1)   {
                switch (opt) {
                    case 0:
                        if (long_options[option_index].flag != 0)
                            break;
                    case 'h':
                        help_flag = 1;
                        break;
                    default:
                        wrong_options=1;

                }
            } else
            {
                wrong_options = 1; // only help is possible
                optind++;
            }
        }
    }

    optind = 1;

    // show help if options are wrong or user want it
    if (wrong_options || help_flag) printf("%s \n", help);
    else
    {
        cp_ptr = getcwd(cwd_bufer, 1024);
        printf("%s \n", cp_ptr);
    }

    return 0;
}

int shell_cd(int argc, const char * argv[]){
    static char help[] = "cd <path> [-h|--help]  -- перейти до шляху <path> ";
    int wrong_options = 0;
    help_flag = 0;
    int option_index = 0;
    int opt;


    static struct option long_options[] =
            {
                    {"help", no_argument, &help_flag, 1}
            };


    if (argc > 2) wrong_options=1;
    else {

        while (optind < argc) {
            if ((opt = getopt_long(argc, argv, "h::", long_options, &option_index)) != -1) {
                switch (opt) {
                    case 0:
                        if (long_options[option_index].flag != 0)
                            break;
                    case 'h':
                        help_flag = 1;
                        break;
                    default:
                        wrong_options=1;

                }
            } else
            {
                if (chdir(argv[optind]) == -1) wrong_options = 1;// if path is not correct (chdir otherwise)
                optind++;
            }
        }
    }

    optind = 1;

    // show help if options are wrong or user want it
    if (wrong_options || help_flag) printf("%s \n", help);

    return 0;
}

int shell_exit(int argc, const char * argv[]) {
    static char help[] = "exit [код завершення] [-h|--help]  – вийти ";
    int wrong_options = 0;
    help_flag = 0;
    int to_exit = 0;
    int exit_code = 0;
    int option_index = 0;
    int opt;
    char *ptr;
    char str_digits[] = "1234567890";

    static struct option long_options[] =
            {
                    {"help", no_argument, &help_flag, 1}
            };


    if (argc > 2) wrong_options=1;
                else {

        while (optind < argc) {
            if ((opt = getopt_long(argc, argv, "h::", long_options, &option_index)) != -1) {
                switch (opt) {
                    case 0:
                        if (long_options[option_index].flag != 0)
                            break;
                    case 'h':
                        help_flag = 1;
                        break;
                    default:
                        wrong_options=1;

                }
            } else {
                // we didn't get exit code option yet
                if (to_exit == 0) {
                    if (strspn(argv[optind], str_digits) == strlen(argv[optind])) {
                        exit_code = (int) strtol(argv[optind], &ptr, 10);
                        to_exit = 1;
                    } else wrong_options = 1; // string is not a number
                }
                else wrong_options = 1; // we can't get exit code twice
                optind++;
            }
        }
    }

    optind = 1;

    // show help if options are wrong or user want it
    if (wrong_options || help_flag)
    {
        printf("%s \n", help);
        return 0;
    }

    exit(exit_code);
}

int export_variable(int argc, const char * argv[]){

    if (argc > 1)
    {
        reti = regexec(&assignRegex, argv[1], 0, NULL, 0);
        if (!reti) {
            assign_variable(argv[1]);
            putenv(argv[1]);
        } else
        {
            for (int i = 0; i < number_of_variables; i++){
                if (strcmp(list_of_variables[i].key, argv[1]) == 0){
                    char *environment_variable;
                    strcpy(environment_variable, list_of_variables[i].key);
                    strcat(environment_variable, "=");
                    strcat(environment_variable, list_of_variables[i].value);
                    putenv(environment_variable);
                    break;
                }
            }
        }
    }

    return 0;
}

int assign_variable(char * line){
    pair key_value;

    for (int i = 0; i < sizeof(line); i++)
    {
        if (line[i] == '=')
        {
            key_value.key = (char *) malloc(i);
            memcpy(key_value.key, line, i);
            key_value.value = (char *) malloc(sizeof(line)-i-1);
            memcpy(key_value.value, &line[i+1], sizeof(line)-i-1);
            break;
        }
    }

    list_of_variables[number_of_variables] = key_value;
    number_of_variables++;

    return 0;
}

char ** substitute_variables(int argc, const char * argv[]) {
    char ** substituted_argv = malloc(argc*sizeof(char *));
    char name_of_variable[256];
    char * value;

    for (int i = 0; i < argc; i++){
        if (argv[i][0] == '$') {
            memcpy(name_of_variable, &argv[i][1], sizeof(argv[i])-1);
            name_of_variable[sizeof(argv[i])-1] ='\0';

            for (int j = 0; j < number_of_variables; j++) {
                if (strcmp(name_of_variable, list_of_variables[j].key) == 0) {
                    value = list_of_variables[j].value;
                    break;
                }
            }
            if (value == NULL) {
                value = getenv(name_of_variable);
            }
            if (value == NULL) {
                value = argv[i];
            }
        } else value = argv[i];
        substituted_argv[i] = value;
    }
    return substituted_argv;
}

#define LSH_TOK_BUFSIZE 64
#define LSH_TOK_DELIM " \t\r\n\a"
char ** split_line(char * line) {
    int buffer_size = LSH_TOK_BUFSIZE, position = 0;
    char ** tokens = malloc(buffer_size * sizeof(char*));
    char * token;

    if (!tokens) {
        fprintf(stderr, "shell: allocation error\n");
        exit(EXIT_FAILURE);
    }

    token = strtok(line, LSH_TOK_DELIM);

    while (token != NULL) {
        tokens[position] = token;
        position++;

        if (position >= buffer_size) {
            buffer_size += LSH_TOK_BUFSIZE;
            tokens = realloc(tokens, buffer_size * sizeof(char*));
            if (!tokens) {
                fprintf(stderr, "split_line: allocation error\n");
                exit(EXIT_FAILURE);
            }
        }
        token = strtok(NULL, LSH_TOK_DELIM);
    }

    tokens[position] = NULL;
    return tokens;
}

char * read_line() {
    char * line;
    size_t buffer_size = 0;
    getline(&line, &buffer_size, stdin);
    return line;
}

int count_argv(char** argv){
    int argc = 0;
    while (argv[argc] != NULL){
        argc++;
    }
    return argc;
}

int remove_sharp_from_line(char * line)
{
    for (int i = 0; i < sizeof(line); i++)
    {
        if (line[i] == '#')
        {
            line[i] = NULL;
            break;
        }
    }

    return 0;
}


int external_execute(int argc, const char * argv[]) {
    pid_t childPid;
    int execution_return;

    switch (childPid = fork()) {
        case -1:
            /*handle error*/
        case 0:
            /*perform action specific to child*/
            {
                execution_return = execvp(argv[0], argv);
            }
        default:
            if (wait(NULL) == -1){
                printf("%s \n", "something wrong");
            }
    }

    return execution_return;
}


int execute_script(int argc, const char *argv[]);

int execute(int argc, const char *argv[]) {

    if (argc == 0) {
        return 1;
    }
    if (strcmp(argv[0], "cd") == 0) {
        return shell_cd(argc, argv);
    }
    if (strcmp(argv[0], "pwd") == 0) {
        return shell_pwd(argc, argv);
    }
    if (strcmp(argv[0], "exit") == 0) {
        return shell_exit(argc, argv);
    }
    if (strcmp(argv[0], "export") == 0) {
        return export_variable(argc, argv);
    }

    reti = regexec(&scriptRegex, argv[0], 0, NULL, 0);

    if (!reti) {
        return execute_script(argc, argv);
    }

    reti = regexec(&assignRegex, argv[0], 0, NULL, 0);

    if (!reti) {
        return assign_variable(argv[0]);
    }

    return external_execute(argc, argv);
}

int execute_script(int argc, const char *argv[]){
    char ** local_argv;
    char * local_line = NULL;
    int local_argc;
    ssize_t read;
    size_t len = 0;

    char filename[20];
    memcpy(filename, &argv[0][2], sizeof(argv[0]));
    FILE *file;
    file = fopen(filename, "r");

    if (file == NULL)
    {
        printf("%s %s\n", "There is no such file:", filename);
    } else if (file) {
        while ((read = getline(&local_line, &len, file)) != -1) {
            local_argv = split_line(local_line);
            local_argc = count_argv(local_argv);

            execute(local_argc, local_argv);

            free(local_line);
            free(local_argv);
        }
    }
}




int loop(){
    char * line;
    char ** argv;
    char *cp_ptr;
    int argc;

    while (1) {
        cp_ptr = getcwd(cwd_bufer, 100);
        // make path shorter to improve user experience
        if (strlen(cp_ptr) > 30){
            cp_ptr = cp_ptr + strlen(cp_ptr) - 30;
            printf("%s%-30s", "...", cp_ptr);
        } else printf("%-33s", cp_ptr);
        printf("$ ");




        line = read_line();
        remove_sharp_from_line(line);
        argv = split_line(line);
        argc = count_argv(argv);
        argv = substitute_variables(argc, argv);

        execute(argc, argv);


        free(line);
        free(argv);
    }



}

int main(){
    int exit_code;
    char *original_path;
    char environment_var[1024] = "PATH=";
    list_of_variables = malloc(256 * sizeof(pair));

    reti = regcomp(&scriptRegex, "^[.][/]", 0);
    reti = regcomp(&assignRegex, "=", 0);

    original_path = getcwd(cwd_bufer, 1024);
    strcat(environment_var, original_path);
    putenv(environment_var);

    exit_code = loop();

    return exit_code;
}