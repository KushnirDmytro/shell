// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <getopt.h>
#include <string.h>

static int help_flag = 0;
static int is_dir_name = 0;
static int wrong_options = 0;
static char *help = "mkdir [-h|--help]  <dirname> – створити директорію";
struct stat st = {0};


static struct option long_options[] =
        {
                {"help", no_argument, &help_flag, 1},
        };


int main(int argc, const char* argv[])
{
        int opt;
        int option_index;
        char dir_name[256];

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

                }
            } else {
                // we didn't get name of dir yet
                if (is_dir_name == 0) {
                    is_dir_name = 1;
                    strcpy(dir_name, argv[optind]);
                }
                else wrong_options = 1; // we can't get name of dir twice
                optind++;
            }
        }
    }

    optind = 1;

    // show help if options are wrong or user want it
    if (wrong_options || help_flag || !is_dir_name)
    {
        printf("%s \n", help);
        return 0;
    }

    if (stat(dir_name, &st) == -1) {
        mkdir(dir_name, 0700);
    } else
    {
        printf("%s \n", "Cannot create directory. File exists");
        return 0;
    }

}


