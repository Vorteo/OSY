#include "functions.h"

int main(int argc, char* argv[])
{

    DIR* directory = NULL;
    struct dirent* myfile = NULL;
    struct stat statbuf;
    char path[256];

    if(argc != 2)
    {

        directory = opendir(".");

    }
    else
    {

        directory = opendir(argv[1]);

    }
    if(directory == NULL)
    {
        if((stat(argv[1], &statbuf)) == 0)
        {
            print_ls(statbuf, argv[1]);           
            printf("\n");

        }
        else
        {

            printf("Nejde otevrit nebo neexistuje %s \n", argv[1]);

        }

        return(0);       

    }

    while((myfile = readdir(directory)) != NULL)
    {
        if(argc == 2)
        {

            snprintf(path,sizeof(path),"%s%s",argv[1], myfile->d_name);

        }
        else
        {
            strcpy(path, myfile->d_name);
        }
        if((stat(path, &statbuf)) == 0)
        {
            print_ls(statbuf, myfile->d_name);

            printf("\n");

        }     

    }
    closedir(directory);   
}

