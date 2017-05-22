// This is a personal academic project. Dear PVS-Studio, please check it.

// PVS-Studio Static Code Analyzer for C, C++ and C#: http://www.viva64.com
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
#include <fcntl.h>
#define PERMS 0666 /* RW for owner, group, others */

static int help_flag = 0;
static int force_rewrite = 0;
static int wrong_options = 0;
static char *help = "cp [-h|--help] [-f] <file1> <file2> \n cp [-h|--help] [-f]  <file1> <file2> <file3>... <dir>";
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
    char buf[BUFSIZ];
    int n;
    char ** files = malloc(256 * sizeof * files);
    char * final_directory;
    char * final_file;
    int number_of_files = 0;
    int is_final_directory = 0;
    int is_final_file = 0;


    while (optind < argc) {
        if ((opt = getopt_long(argc, argv, "h::f::", long_options, &option_index)) != -1) {
            switch (opt) {
                case 0:
                    if (long_options[option_index].flag != 0)
                        break;
                case 'h':
                    help_flag = 1;
                    break;
                case 'f':
                    force_rewrite = 1;
                    break;
                default:
                    wrong_options = 1;
                    break;
            }
        }
        else {
            while(optind < argc) {
                if (!wrong_options && !help_flag) {
                    if (is_dir(argv[optind]))
                    {
                        if (is_final_directory == 0)
                        {
                            is_final_directory = 1;
                            final_directory = argv[optind];
                        }
                    } else
                    if (is_file(argv[optind])){
                        files[number_of_files] = argv[optind];
                        number_of_files++;
                    }
                    else
                    {
                        is_final_file = 1;
                        final_file = argv[optind];
                    }
                }
                optind++;
            }
        }
    }

    if (number_of_files > 2 && is_final_directory == 0) wrong_options = 1;
    if (is_final_directory && is_final_file) wrong_options = 1;
    if (number_of_files == 1 && !is_final_file && !is_final_directory) wrong_options = 1;
    if (number_of_files > 1 && is_final_file) wrong_options = 1;
    // show help if options are wrong or user want it
    if (wrong_options || help_flag)
    {
        printf("%s \n", help);
        return 0;
    }
    else
    {
        if (number_of_files == 1 && is_final_file)
        {
            int sourcefd;
            int destfd;
            if ((sourcefd = open(files[0], O_RDONLY, 0)) == -1)
            {
                printf("cp: can't open %s\n", files[0]);
            }
            else
            if ((destfd = creat(final_file, PERMS)) == -1)
            {
                printf("cp: can't create %s, mode %03o\n",
                        final_file, PERMS);
            } else
            {
                while ((n = read(sourcefd, buf, BUFSIZ)) > 0)
                    if (write(destfd, buf, n) != n)
                        printf("cp: write error on file %s", final_file);
            }
        }
        // we need to rewrite file
        else if (number_of_files == 2 && is_final_directory == 0)
        {
            int sourcefd;
            int destfd;
            if ((sourcefd = open(files[0], O_RDONLY, 0)) == -1)
            {
                printf("cp: can't open %s\n", files[0]);
            }
            else {
                if (is_file(files[1]))
                {
                    if (force_rewrite)
                        unlink(files[1]);
                    else
                    {
                        char str[2];
                        printf("%s %s %s", "Are you sure want to rewrite ", files[1], "(y/n)");
                        scanf("%1s", str);
                        if (strcmp(str, "Y") == 0 || strcmp(str, "y") == 0)
                            unlink(files[1]);
                    }
                }
                if ((destfd = creat(files[1], PERMS)) == -1) {
                    printf("cp: can't create %s, mode %03o\n",
                           final_file, PERMS);
                } else {
                    while ((n = read(sourcefd, buf, BUFSIZ)) > 0)
                        if (write(destfd, buf, n) != n)
                            printf("cp: write error on file %s", files[1]);
                }
            }

        }
        else if (is_final_directory)
        {
            for (int i = 0; i < number_of_files; i++)
            {
                int sourcefd;
                int destfd;
                char final_dir_file[100];
                strcpy(final_dir_file, final_directory);
                strcat(final_dir_file, "/");
                strcat(final_dir_file, files[i]);
                if ((sourcefd = open(files[i], O_RDONLY, 0)) == -1)
                {
                    printf("cp: can't open %s\n", files[0]);
                }
                else {
                    if (is_file(final_dir_file))
                    {
                        if (force_rewrite)
                        unlink(final_dir_file);
                        else
                        {
                            char str[2];
                            printf("%s %s %s", "Are you sure want to rewrite ", final_dir_file, "(y/n)");
                            scanf("%1s", str);
                            if (strcmp(str, "Y") == 0 || strcmp(str, "y") == 0)
                                unlink(final_dir_file);
                        }
                    }

                    if ((destfd = creat(final_dir_file, PERMS)) == -1) {
                        printf("cp: can't create %s, mode %03o\n",
                               final_file, PERMS);
                    } else {
                        while ((n = read(sourcefd, buf, BUFSIZ)) > 0)
                            if (write(destfd, buf, n) != n)
                                printf("cp: write error on file %s", final_dir_file);
                    }
                }
            }
        }
    }

    return 0;
}


