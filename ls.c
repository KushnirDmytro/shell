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


static regex_t regex;
static int reti;

static int help_flag = 0;
static int information_flag = 0;
static int sort_flag = 0;
static char *sort_value = NULL;
static int reverse_flag = 0;
static int wrong_options = 0;
static int wrong_sort_opt = 0;
static int path_or_mask = 0;
static char *path_and_mask = NULL;
static char *help = "ls [path|mask] [-I] [-h|--help] [--sort=U|S|t|X] [-r] – вивести список файлів";

static struct option long_options[] =
        {
                {"help", no_argument, &help_flag, 1},
                {"sort", optional_argument, &sort_flag, 1}
        };


 struct my_files {
    char name[256];
    struct stat file_information[256];
};

typedef  struct my_files my_files;


static int last_modification_cmp(const void *filea, const void *fileb)
{
    const my_files *dfilea = filea, *dfileb = fileb;

    return dfileb->file_information->st_mtime - dfilea->file_information->st_mtime;
   // return dfilea->file_information->st_mtime   > dfileb->file_information->st_mtime? -1 : dfilea->file_information->st_mtime   < dfileb->file_information->st_mtime;
}

static int name_cmp(const void *filea, const void *fileb)
{
    const struct my_files *dfilea = filea, *dfileb = fileb;

    return strcmp(dfilea->name, dfileb->name);
}

static int size_cmp(const void *filea, const void *fileb)
{
    const struct my_files *dfilea = filea, *dfileb = fileb;

    return dfilea->file_information->st_size  > dfileb->file_information->st_size? -1 : dfilea->file_information->st_size   < dfileb->file_information->st_size;
}

static int ext_cmp(const void *filea, const void *fileb)
{
    const struct my_files *dfilea = filea, *dfileb = fileb;
    char * exta = strchr(dfilea->name, '.');
    char * extb= strchr(dfileb->name, '.');
    if (exta == NULL) exta = "";
    if (extb == NULL) extb = "";
    return strcmp(exta, extb);
}

int is_dir(const char* path) {
    struct stat st = {0};
    int err = stat(path, &st);

    if (err == -1)
        return 0;
    if (S_ISDIR(st.st_mode)) return 1;

    return 0;
}

int main(int argc, const char* argv[])
{
    int opt;
    int option_index;

   struct my_files* list_of_files = malloc(256 * sizeof(my_files) );


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
                                if (strcmp(sort_value, "U") != 0 &&
                                    strcmp(sort_value, "S") != 0 &&
                                    strcmp(sort_value, "t") != 0 &&
                                    strcmp(sort_value, "X") != 0 &&
                                    strcmp(sort_value, "N") != 0)
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
                reti = regcomp(&regex, path_and_mask, 0);
                if (is_dir(path_and_mask) || !reti) {
                    path_or_mask = 1;
                }
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
        if (path_or_mask) {
            if (is_dir(path_and_mask))
            {
                path_or_mask = 0;
                d = opendir(path_and_mask);
            }
            else
            {
                d = opendir(".");
            }
        }
        else d = opendir(".");

        int number_of_files = 0;

        if (d)
        {
            while ((dir = readdir(d)) != NULL)
            {
                if (strcmp(dir->d_name, ".") != 0 && strcmp(dir->d_name, "..")) {
                    stat(dir->d_name, list_of_files[number_of_files].file_information);
                    strcpy(list_of_files[number_of_files].name, dir->d_name);
                    number_of_files++;
                }
            }

            closedir(d);
        }


        if (sort_flag){
            if (strcmp(sort_value, "S") == 0)
            {qsort(list_of_files, number_of_files, sizeof(struct my_files), size_cmp);}
            else if (strcmp(sort_value, "N") == 0)
            {qsort(list_of_files, number_of_files, sizeof(struct my_files), name_cmp);}
            else if (strcmp(sort_value, "t") == 0)
            {qsort(list_of_files, number_of_files, sizeof(struct my_files), last_modification_cmp);}
            else if (strcmp(sort_value, "X") == 0)
            {qsort(list_of_files, number_of_files, sizeof(struct my_files), ext_cmp);}
        }
        else qsort(list_of_files, number_of_files, sizeof(struct my_files), name_cmp);

            int file_number;
            for (int j = 0; j < number_of_files; j++)
            {
                if (reverse_flag) file_number = number_of_files - j - 1;
                else file_number = j;


                if (path_or_mask) {
                    reti = regexec(&regex, list_of_files[file_number].name, 0, NULL, 0);
                } else reti = 0;
                        if(!reti)
                        {
                        if (information_flag) {
                            printf("%-20s %-10i %-20s\n", list_of_files[file_number].name,
                                   list_of_files[file_number].file_information->st_size,
                                   ctime(&list_of_files[file_number].file_information->st_mtime));
                        } else printf("%-20s\n", list_of_files[file_number].name);
                    }
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
