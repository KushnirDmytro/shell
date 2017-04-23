#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <getopt.h>
#include <string.h>

static int help_flag = 0;
static int information_flag = 0;
static int sort_flag = 0;
static char *sort_value = NULL;
static int reverse_flag = 0;
static int wrong_options = 0;
static int wrong_sort_opt = 0;
static int path_or_mask = 0;
static char *path_and_mask = NULL;
static char *path = ".";
static char *mask = NULL;
static char *help = "ls [path|mask] [-I] [-h|--help] [--sort=U|S|t|X] [-r] – вивести список файлів";

static struct option long_options[] =
        {
                {"help", no_argument, &help_flag, 1},
                {"sort", optional_argument, &sort_flag, 1}
        };


int main(int argc, const char* argv[])
{
    int opt;
    int option_index;
    char ls_output[256][256];


    while (optind < argc) {
        if ((opt = getopt_long(argc, argv, "I::r::h::", long_options, &option_index)) != -1)   {
            switch (opt) {
                case 0:
                    if (long_options[option_index].flag != 0)
                        if (strcmp(long_options[option_index].name, "sort") == 0)
                        {
                            // check if sort arg is correct
                            if (optarg)
                            {
                                sort_value = optarg;
                                if (strcmp(sort_value, "U") > 0 &&
                                    strcmp(sort_value, "S") > 0 &&
                                    strcmp(sort_value, "t") > 0 &&
                                    strcmp(sort_value, "X") > 0)
                                {
                                    wrong_sort_opt = 1;
                                }
                            }
                            else wrong_sort_opt = 1;

                        }
                    break;
                case 'h':
                    help_flag = 1;
                    break;
                case 'I':
                    information_flag = 1;
                    break;
                case 'r':
                    reverse_flag = 1;
                    break;
                default:
                    wrong_options=1;
            }
        } else {
            if (path_or_mask == 0) {
                path_and_mask = argv[optind];
                path_or_mask = 1;
            }
            else
                wrong_options = 1;
            optind++;
        }
    }

    if (wrong_options || help_flag || wrong_sort_opt) {
        printf("%s \n", help);
    } else {

        DIR           *d;
        struct dirent *dir;
        d = opendir(".");
        int i = 0;
        if (d)
        {
            while ((dir = readdir(d)) != NULL)
            {
                strcpy(ls_output[i], dir->d_name);
                i++;
            }

            closedir(d);
        }
        for (int j = 0; j < i; j++){
            printf("%s\n", ls_output[j]);
        }
    }
    return 0;
}


/*DIR *mydir;
struct dirent *myfile;
struct stat mystat;

mydir = opendir(".");
while((myfile = readdir(mydir)) != NULL)
{
    stat(myfile->d_name, &mystat);
    printf("%d",mystat.st_size);
    printf(" %s\n", myfile->d_name);
}
closedir(mydir);*/
