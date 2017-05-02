#define _XOPEN_SOURCE 500
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <getopt.h>
#include <string.h>
#include <ftw.h>


static int help_flag = 0;
static int force_delete = 0;
static int recursive_delete = 0;
static int wrong_options = 0;
static char *help = "rm [-h|--help] [-f] [-R] <file1> <file2> <file3> – видалити";
struct stat st = {0};

static struct option long_options[] =
        {
                {"help", no_argument, &help_flag, 1},
        };


int delete_dir(const char* dirname) {
    return nftw(dirname, remove, 64, FTW_DEPTH | FTW_PHYS);
}

int delete_file(const char* filename) {
    return unlink(filename);
}

int is_dir(const char* path) {
    struct stat st = {0};
    int err = stat(path, &st);

    if (err == -1)
        return 0;
    if (S_ISDIR(st.st_mode)) return 1;

    return 0;
}

int is_file(const char* path) {
    FILE *file;
    if (file = fopen(path, "r"))
    {
        fclose(file);
        return 1;
    }
    return 0;
}

int main(int argc, const char* argv[])
{
    int opt;
    int option_index;


        while (optind < argc) {
            if ((opt = getopt_long(argc, argv, "h::R::f::", long_options, &option_index)) != -1) {
                switch (opt) {
                    case 0:
                        if (long_options[option_index].flag != 0)
                            break;
                    case 'h':
                        help_flag = 1;
                        break;
                    case 'R':
                        recursive_delete = 1;
                        break;
                    case 'f':
                        force_delete = 1;
                        break;
                    default:
                        wrong_options = 1;
                        break;
                }
            }
            else {
                while(optind < argc) {
                    if (!wrong_options && !help_flag) {
                        if (!force_delete) {

                            char str[2];
                            printf("%s %s %s", "Are you sure want to delete ", argv[optind], "(y/n)");
                            scanf("%1s", str);
                            if (strcmp(str, "Y") == 0 || strcmp(str, "y") == 0) {
                                if (is_dir(argv[optind]) && recursive_delete) {
                                    delete_dir(argv[optind]);
                                } else if (is_file(argv[optind])) {
                                    delete_file(argv[optind]);
                                }
                            }
                        }else
                            if (is_dir(argv[optind]) && recursive_delete) {
                                delete_dir(argv[optind]);
                            } else if (is_file(argv[optind])) {
                                delete_file(argv[optind]);
                            }
                    }
                    optind++;
                }
            }
        }


    // show help if options are wrong or user want it
    if (wrong_options || help_flag)
    {
        printf("%s \n", help);
        return 0;
    }


}


