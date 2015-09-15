#include <unistd.h>
#include <stdio.h>
#include <dirent.h>
#include <string.h>
#include <sys/stat.h>

//input needs to be absolute path
void printdir(char *dir)
{
    DIR *dp;
    struct dirent *entry;
    struct stat statbuf;

    if ((dp = opendir(dir)) == NULL) {
        fprintf(stderr, "cannot open directory: %s\n", dir);
        return;
    }
    if (chdir(dir) != 0)
        printf("err chdir");
    while ((entry = readdir(dp)) != NULL) {
        lstat(entry->d_name, &statbuf);
        if (S_ISDIR(statbuf.st_mode)) {

            if (strcmp(".", entry->d_name) == 0 || strcmp("..", entry->d_name) == 0)
                continue;

            char newdir[500];
            strcpy(newdir, dir);
            strcat(newdir, "/");
            strcat(newdir, entry->d_name);
            printdir(newdir);
        }
        else if (strcmp(dir, "") != 0)
        {
            printf("%s/%s\n", dir, entry->d_name);
        }
    }

    chdir("..");
    closedir(dp);
}

int main(int argc, char* argv[])
{
    char pwd[2] = ".";
    char topdir[500] = "";
    // char curdir[500] = "";
    if (argc != 2)
        strcpy(topdir, pwd);
    else if (argv[1][0] != '/')
    {
        realpath(argv[1], topdir);
    }
    printf("%s\n", topdir);
    printdir(topdir);

    return 0;
}
