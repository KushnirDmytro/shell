#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <dirent.h>
#include <getopt.h>
#include <string.h>
#include <regex.h>

static regex_t regex;
static int reti;
char msgbuf[100];

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


struct my_files {
    char name[256];
    struct stat file_information[256];
};


static int last_modification_cmp(const void *filea, const void *fileb)
{
    const struct my_files *dfilea = filea, *dfileb = fileb;

    return dfilea->file_information->st_mtime   > dfileb->file_information->st_mtime? -1 : dfilea->file_information->st_mtime   < dfileb->file_information->st_mtime;
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
    char * exta = strtok(dfilea->name, '.');
    char * extb = strtok(dfileb->name, '.');

    char * last_exta;

    while (exta != NULL){
        last_exta = exta;
        exta = strtok(NULL, '.');
    }

    char * last_extb;

    while (extb != NULL){
        last_extb = extb;
        extb = strtok(NULL, '.');
    }

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

   struct my_files* list_of_files = malloc(256 * sizeof *list_of_files);


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
                stat(dir->d_name, list_of_files[number_of_files].file_information);
                strcpy(list_of_files[number_of_files].name, dir->d_name);
                number_of_files++;
            }

            closedir(d);
        }

        sort_flag = 1;
        sort_value = "S";
        if (sort_flag){
            if (strcmp(sort_value, "S") == 0)
            {qsort(list_of_files, number_of_files, sizeof(struct my_files *), size_cmp);}
            else if (strcmp(sort_value, "N") == 0)
            {qsort(list_of_files, number_of_files, sizeof(struct my_files *), name_cmp);}
            else if (strcmp(sort_value, "t") == 0)
            {qsort(list_of_files, number_of_files, sizeof(struct my_files *), last_modification_cmp);}
            else if (strcmp(sort_value, "X") == 0)
            {qsort(list_of_files, number_of_files, sizeof(struct my_files *), ext_cmp);}
        }
        else qsort(list_of_files, number_of_files, sizeof(struct my_files *), name_cmp);

        if (reverse_flag)
        {
            for (int j = number_of_files - 1; j >= 0; j--)
            {
                if (path_or_mask) {
                        reti = regexec(&regex, list_of_files[j].name, 0, NULL, 0);
                        if(!reti)
                        {
                        if (information_flag) {
                            printf("%s %i %i\n", list_of_files[j].name,
                                   list_of_files[j].file_information->st_size,
                                   list_of_files[j].file_information->st_mtime);
                        } else printf("%s\n", list_of_files[j].name);
                    }
                } else
                {
                    if (information_flag) {
                        printf("%s %i %i\n", list_of_files[j].name,
                               list_of_files[j].file_information->st_size,
                               list_of_files[j].file_information->st_mtime);
                    } else printf("%s\n", list_of_files[j].name);
                }
            }
        }
        else {
            for (int j = 0; j < number_of_files; j++){
                if (path_or_mask) {
                    reti = regexec(&regex, list_of_files[j].name, 0, NULL, 0);
                    if(!reti)
                    {
                        if (information_flag) {
                            printf("%s %i %i\n", list_of_files[j].name,
                                   list_of_files[j].file_information->st_size,
                                   list_of_files[j].file_information->st_mtime);
                        } else printf("%s\n", list_of_files[j].name);
                    }
                } else
                {
                    if (information_flag) {
                        printf("%s %i %i\n", list_of_files[j].name,
                               list_of_files[j].file_information->st_size,
                               list_of_files[j].file_information->st_mtime);
                    } else printf("%s\n", list_of_files[j].name);
                }
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
