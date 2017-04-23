#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <getopt.h>
#include <sys/wait.h>
#include <sys/stat.h>
#include <fcntl.h>


int shell_pwd(int argc, const char * argv[]);
int shell_cd(int argc, const char * argv[]);
int shell_exit(int argc, const char * argv[]);
static int help_flag = 0;
char *cwd_bufer;

char *external_commands[] = {
    "ls",
    "mkdir"
};
char *external_commands_paths[] = {
    "/home/baget/CLionProjects/at_shell/cmake-build-debug/at_ls",
    "/home/baget/CLionProjects/at_shell/cmake-build-debug/mkdir_ls"
};

long path_max;

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

int external_execute(int argc, const char * argv[]) {
    pid_t childPid;
    int execution_return;

    switch (childPid = fork()) {
        case -1:
            /*handle error*/
        case 0:
            /*perform action specific to child*/
            for (int i = 0; i < sizeof(external_commands); i++){
                if (strcmp(external_commands[i], argv[0]) == 0)
                {
                     execution_return = execv(external_commands_paths[i], argv);
                }
            }
        default:
            if (wait(NULL) == -1){
                printf("%s \n", "some wrong");
            }
    }

    return execution_return;
}

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

    return external_execute(argc, argv);
}

int count_argv(char** argv){
    int argc = 0;
    while (argv[argc] != NULL){
        argc++;
    }
    return argc;
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
            printf("%s", "...");
        }
        printf("%s ", cp_ptr);



        line = read_line();
        argv = split_line(line);
        argc = count_argv(argv);

        execute(argc, argv);


        free(line);
        free(argv);
    }



}

int main(){
    int exit_code;

    exit_code = loop();

    return exit_code;
}