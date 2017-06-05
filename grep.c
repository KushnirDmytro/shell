// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com#include <sys/types.h>
#include <sys/stat.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <getopt.h>
#include <string.h>
#include <regex.h>
#include <time.h>
#include <ctype.h>
#include <zconf.h>
#include <fcntl.h>

static regex_t regex;
static int reti;

static int help_flag = 0;
static int inversion_flag = 0;
static int ignore_flag = 0;
static int wrong_options = 0;
static char *string = NULL;
static char *filename = NULL;
static char *help = "grep [-h|--help] [-v|--invert-match] [-i|--ignore-case] [--file=<filename>] string";

static struct option long_options[] =
        {
                {"help", no_argument, &help_flag, 1},
                {"invert-match", no_argument, &inversion_flag, 1},
                {"ignore-case", no_argument, &ignore_flag, 1},
                {"file", required_argument, 0, 'f'}
        };


int main(int argc, const char* argv[])
{
    int file;
    int opt;
    int fd;
    int option_index;

    while (optind < argc) {
        if ((opt = getopt_long(argc, argv, "i::v::h::f", long_options, &option_index)) != -1)   {
            switch (opt) {
                case 0:
                    if (long_options[option_index].flag != 0)
                        break;
                    if (optarg) filename = optarg;
                    else wrong_options = 1;
                    break;
                case 'i':
                    ignore_flag = 1;
                    break;
                case 'v':
                    inversion_flag = 1;
                    break;
                case 'h':
                    help_flag = 1;
                    break;
                case 'f':
                    if (argv[optind]) {
                        filename = argv[optind];
                        optind++;
                    }
                    else wrong_options = 1;
                    break;
                default:
                    wrong_options=1;
            }
        } else {
            if (string == NULL)
            string = argv[optind];
            else strcat(string, argv[optind]);
            optind++;
        }
    }

    if (filename) {
        file = open(filename, O_RDONLY);
        if (!file) wrong_options = 1;
    }

    if (ignore_flag) {
        for (int i = 0; string[i]; i++) {
            string[i] = tolower((unsigned char) string[i]);
        }
    }

    if (string) {
        reti = regcomp(&regex, string, 0);
        if (reti) {
            wrong_options = 1;
        }
    } else wrong_options = 1;

    if (wrong_options || help_flag) {
        printf("%s \n", help);

    } else
    {
        if (filename) {
            dup2(file, 0);
        }
        char * line = malloc(1000);
        while (scanf("%s", line) != EOF)
        {
            if (strcmp(line, "exitGREP") == 0) {
                return 0;
            }

            if (ignore_flag) {
                char * lowercase = malloc(sizeof(line));
                strcpy(lowercase, line);
                for (int i = 0; lowercase[i]; i++) {
                    lowercase[i] = tolower((unsigned char) lowercase[i]);
                }
                reti = regexec(&regex, lowercase, 0, NULL, 0);
            } else reti = regexec(&regex, line, 0, NULL, 0);

            if ((!reti && !inversion_flag) || (reti && inversion_flag)) {
                printf("%s\n", line);
            }
        }
    }

    return 0;
}