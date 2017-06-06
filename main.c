// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include <regex.h>
#include <errno.h>
#include <fcntl.h>
#include <zconf.h>

static regex_t scriptRegex;
static regex_t assignRegex;
static int reti;

int shell_pwd(int argc, const char * argv[], int out_fd);
int shell_cd(int argc, const char * argv[]);
int shell_exit(int argc, const char * argv[]);
static int help_flag = 0;



struct pair {
    char * key;
    char * value;
};

struct special_argv {
    char *** list_of_argv;
    int * list_of_argc;
    int len;
};

typedef struct pair pair;
typedef struct special_argv special_argv;

static pair * list_of_variables;
static int number_of_variables = 0;


int shell_pwd(int argc, const char * argv[], int out_fd){
    static char help[] = "pwd [-h|--help] – вивести поточний шлях\n";
    char *cwd_bufer = malloc(PATH_MAX);
    int wrong_options = 0;
    help_flag = 0;
    int option_index = 0;
    int opt;

    char *cp_ptr = malloc(PATH_MAX);

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
    if (wrong_options || help_flag)
    {
        write(out_fd, help, strlen(help));
    }
    else
    {
        cp_ptr = getcwd(cwd_bufer, 1024);
        strcat(cp_ptr, "\n");
        write(out_fd, cp_ptr, strlen(cp_ptr));
    }

    if (out_fd != STDOUT_FILENO)
    {
        close(out_fd);
    }

    return 0;
}

int shell_cd(int argc, const char * argv[]){
    static char help[] = "cd <path> [-h|--help]  -- перейти до шляху <path>\n";
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
    static char help[] = "exit [код завершення] [-h|--help]  – вийти\n ";
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

int shell_echo(int argc, const char * argv[], int out_fd) {

    for (int i = 0; i < argc; i++) {
        write(out_fd, argv[i], strlen(argv[i]));
        write(out_fd, " ", strlen(" "));
    }
    write(out_fd, "\n", strlen("\n"));

    if (out_fd != STDOUT_FILENO) close(out_fd);

    return 0;
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
                    char *environment_variable = malloc(1024);
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

    for (int i = 0; i < strlen(line); i++)
    {
        if (line[i] == '=')
        {
            key_value.key = (char *) malloc(i+1);
            memcpy(key_value.key, line, i);
            key_value.value = (char *) malloc(sizeof(line)-i);
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


    for (int i = 0; i < argc; i++){
        char * value = NULL;
        if (argv[i][0] == '$') {
            memcpy(name_of_variable, &argv[i][1], strlen(argv[i])-1);
            name_of_variable[strlen(argv[i])-1] ='\0';

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
        substituted_argv[i] = malloc(sizeof(value)+1);
        strcpy(substituted_argv[i], value);
    }
    substituted_argv[argc] = NULL;
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

// here we group all our programm calls and arguments for future pipeline
struct special_argv split_subcommands(int argc, char ** argv){
    int number_of_subbcomands = 1;
    special_argv subcommands;

    for (int i = 0; i< argc; i++ ){
        if (strcmp(argv[i],"|") == 0) number_of_subbcomands++;
    }


    subcommands.len = number_of_subbcomands;
    subcommands.list_of_argc = calloc(argc, sizeof(int));
    subcommands.list_of_argv = malloc((argc+1)*sizeof(char **));
    subcommands.list_of_argv[0] = malloc((argc+1) * sizeof(char *));
    int index = 0;
    for (int i = 0; i < argc; i++ ) {
        if (strcmp(argv[i], "|") == 0){
            subcommands.list_of_argv[index][subcommands.list_of_argc[index]+1] = NULL;
            index++;
            subcommands.list_of_argv[index] = malloc((argc+1) * sizeof(char *));
            subcommands.list_of_argc[index] = 0;
            //subcommands.list_of_argv[index] = malloc(argc * sizeof(char ));
        }
        else
        {
            subcommands.list_of_argv[index][subcommands.list_of_argc[index]] = malloc(sizeof(argv[i]));
            subcommands.list_of_argv[index][subcommands.list_of_argc[index]] = strdup(argv[i]);
            //strcpy(list_of_argv[index][argc_subbcomand[index]], argv[i]);
            subcommands.list_of_argc[index]++;
        }
    }



    return subcommands;
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
    for (int i = 0; i < strlen(line); i++)
    {
        if (line[i] == '#')
        {
            line[i] = NULL;
            break;
        }
    }

    return 0;
}

int print_from_pipe(int fd) {
    char buf[BUFSIZ] = {0};
    printf("\n");
    while(read(fd, buf, BUFSIZ) != 0) {
        printf("%s\n", buf);
    }
    return 0;
}

int external_execute(int argc, const char * argv[], int input_fd, int out_fd, int err_fd, int child_to_close, int parent_to_close) {
    pid_t childPid;
    int execution_return = 0;


    switch (childPid = fork()) {
        case -1:
            /*handle error*/
            printf("Error fork, code %d: %s\n", errno, strerror(errno));
            execution_return = -1;
        case 0:
            /*perform action specific to child*/
        {
            dup2(out_fd, 1);
            dup2(input_fd, 0);
            dup2(err_fd, 2);
            if (child_to_close > -1) close(child_to_close);
            execution_return = execvp(argv[0], argv);
            printf("Error executing, code %d: %s\n", errno, strerror(errno));
            exit(-1);
        }
        default: {
            if (parent_to_close > -1) close(parent_to_close);
        }
    }
            //if (out_fd == STDOUT_FILENO){
                //if (wait(NULL) == -1)
                //printf("%s \n", "something wrong");
            //} else close(out_fd);


    return execution_return;
}


int execute_script(int argc, const char *argv[]);

int execute(int argc, const char *argv[], int input_fd, int out_fd, int error_fd, int child_to_close, int parent_to_close) {

    if (argc == 0) {
        return 1;
    }
    if (strcmp(argv[0], "cd") == 0) {
        if (error_fd != STDERR_FILENO) close(error_fd);
        if (input_fd != STDIN_FILENO) close(input_fd);
        if (out_fd != STDOUT_FILENO) close(out_fd);
        return shell_cd(argc, argv);
    }
    if (strcmp(argv[0], "pwd") == 0) {
        if (input_fd != STDIN_FILENO) close(input_fd);
        if (error_fd != STDERR_FILENO) close(error_fd);
        return shell_pwd(argc, argv, out_fd);
    }
    if (strcmp(argv[0], "exit") == 0) {
        if (input_fd != STDIN_FILENO) close(input_fd);
        if (out_fd != STDOUT_FILENO) close(out_fd);
        if (error_fd != STDERR_FILENO) close(error_fd);
        return shell_exit(argc, argv);
    }
    if (strcmp(argv[0], "export") == 0) {
        if (input_fd != STDIN_FILENO) close(input_fd);
        if (out_fd != STDOUT_FILENO) close(out_fd);
        if (error_fd != STDERR_FILENO) close(error_fd);
        return export_variable(argc, argv);
    }
    if (strcmp(argv[0], "echo") == 0) {
        if (input_fd != STDIN_FILENO) close(input_fd);
        if (error_fd != STDERR_FILENO) close(error_fd);
        return shell_echo(argc, argv, out_fd);
    }

    reti = regexec(&scriptRegex, argv[0], 0, NULL, 0);

    if (!reti) {
        return execute_script(argc, argv);
    }

    reti = regexec(&assignRegex, argv[0], 0, NULL, 0);

    if (!reti) {
        return assign_variable(argv[0]);
    }

    return external_execute(argc, argv, input_fd, out_fd, error_fd, child_to_close, parent_to_close);
}

int execute_all (special_argv subbcomands) {
    int out_in_file;
    int err_in_file;
    int input_from_file;
    int out_garbage;
    char filename[256];
    int child_to_close = -1;
    int parent_to_close = -1;
    int pipe_fd[2];
    int input_fd = STDIN_FILENO;
    int out_fd = STDOUT_FILENO;
    int err_fd = STDERR_FILENO;
    int execute_code;

    for (int i = 0; i < subbcomands.len; i++){
        child_to_close = -1;
        parent_to_close = -1;
        out_in_file = 0;
        err_in_file = 0;
        out_garbage = 0;
        input_from_file = 0;
        memset(filename, 0, 256*sizeof(char));

        for (int j = subbcomands.list_of_argc[i]-1; j >= 0; j--){
            if (strcmp(subbcomands.list_of_argv[i][j], ">") == 0) {
                out_in_file = 1;
                strcpy(filename, subbcomands.list_of_argv[i][j+1]);
                subbcomands.list_of_argv[i][j] = NULL;
                subbcomands.list_of_argc[i] = j;
                break;
            } else if (strcmp(subbcomands.list_of_argv[i][j], "<") == 0) {
                input_from_file = 1;
                strcpy(filename, subbcomands.list_of_argv[i][j+1]);
                subbcomands.list_of_argv[i][j] = NULL;
                subbcomands.list_of_argc[i] = j;
                break;
            } else if (strcmp(subbcomands.list_of_argv[i][j], "2>") == 0) {
                err_in_file = 1;
                strcpy(filename, subbcomands.list_of_argv[i][j]);
                subbcomands.list_of_argv[i][j] = NULL;
                subbcomands.list_of_argc[i] = j;
                break;
            } else if (strcmp(subbcomands.list_of_argv[i][j], "2>&1") == 0) {
                out_in_file = 1;
                err_in_file = 1;
                strcpy(filename, subbcomands.list_of_argv[i][j-1]);
                subbcomands.list_of_argv[i][j-2] = NULL;
                subbcomands.list_of_argc[i] = j - 2;
                break;
            } else if (strcmp(subbcomands.list_of_argv[i][j], "&") == 0) {
                out_garbage = 1;
                subbcomands.list_of_argv[i][j] = NULL;
                subbcomands.list_of_argc[i] = j;
                break;
            }
        }

        if (!input_from_file) {
            if (i > 0 && pipe_fd[0] > 0) {
                input_fd = pipe_fd[0];
                child_to_close = pipe_fd[1];
                parent_to_close = pipe_fd[0];
            } else
            {
                input_fd = STDIN_FILENO;
            }
        } else
        {
            input_fd = open(filename, O_RDONLY);
            if (input_fd == -1) {
                perror("opening file");
                exit(EXIT_FAILURE);
            }
        }

        if (!out_in_file) {
            if (out_garbage)
            {
                out_fd = open("/dev/null", 0);
            } else {
                    if (pipe(pipe_fd) == -1) {
                        perror("pipe");
                        exit(EXIT_FAILURE);
                    } else {
                        out_fd = pipe_fd[1];
                        child_to_close = pipe_fd[0];
                        parent_to_close = pipe_fd[1];
                    }
            }
        } else {
            out_fd = open(filename, O_CREAT | O_RDWR, 0666);
            pipe_fd[0] = -1;
        }

        if (!err_in_file) {
            err_fd = STDERR_FILENO;
        } else err_fd = open(filename, O_CREAT | O_RDWR, 0666);

        execute_code = execute(subbcomands.list_of_argc[i], subbcomands.list_of_argv[i], input_fd, out_fd, err_fd, child_to_close, parent_to_close);
        if (execute_code != 0)
        {
            return execute_code;
        }

    }

    if (!out_in_file)
    {
        input_fd = pipe_fd[0];
        print_from_pipe(input_fd);
    }

    return execute_code;
}

int execute_script(int argc, const char *argv[]){
    char ** local_argv;
    char * local_line = NULL;
    int local_argc;
    special_argv local_subcommands;
    ssize_t read;
    size_t len = 0;

    char * filename = malloc(sizeof(argv[0]));
    strcpy(filename, &argv[0][2]);
    FILE *file;
    file = fopen(filename, "r");

    if (file == NULL)
    {
        printf("%s %s\n", "There is no such file:", filename);
    } else if (file) {
        while ((read = getline(&local_line, &len, file)) != -1) {
            local_argv = split_line(local_line);
            local_argc = count_argv(local_argv);
            local_argv = substitute_variables(local_argc, local_argv);
            local_subcommands = split_subcommands(local_argc, local_argv);

            execute_all(local_subcommands);

            free(local_line);
            for (int i = 0; i < local_subcommands.len; i++) {
                for (int j = 0; j < local_subcommands.list_of_argc[i]; j++)
                {
                    free(local_subcommands.list_of_argv[j]);
                    //break;
                }
            }
            //free(local_subcommands.list_of_argc);
            free(local_subcommands.list_of_argv);
            //free(local_argv);
        }
    }
    free(filename);
    return 0;
}


int loop(){
    char * line;
    char ** argv;
    char *cp_ptr = malloc(PATH_MAX+1);
    char *cwd_bufer = malloc(PATH_MAX+1);
    int argc;
    special_argv subcommands;

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
        subcommands = split_subcommands(argc, argv);

        /*for (int i = 0; i < subcommands.len; i++){
            for (int j = 0; j < subcommands.list_of_argc[i]; j++){
                printf("%s ", subcommands.list_of_argv[i][j]);
            }fl
            printf("\n");
        }*/

        execute_all(subcommands);


        free(line);
        for (int i = 0; i < subcommands.len; i++) {
            for (int j = 0; j < subcommands.list_of_argc[i]; j++)
            {
                free(subcommands.list_of_argv[i][j]);
            }
            //free(subcommands.list_of_argv[i]);
        }
        free(subcommands.list_of_argc);
        free(subcommands.list_of_argv);
        //free(argv);
        //free(&subcommands);
    }



}

int main() {
    int exit_code;
    char *original_path;
    char environment_var[1024] = "PATH=";
    char *cwd_bufer = malloc(PATH_MAX+1);
    list_of_variables = malloc(256 * sizeof(pair));

    reti = regcomp(&scriptRegex, "^[.][/]", 0);
    reti = regcomp(&assignRegex, "=", 0);

    original_path = getcwd(cwd_bufer, 1024);
    strcat(environment_var, original_path);
    putenv(environment_var);

    exit_code = loop();

    return exit_code;
}
