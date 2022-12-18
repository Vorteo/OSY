#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <unistd.h>
#include <time.h>
#include <queue>
#include <iostream>
#include <string.h>


int file_is_modified(const char *filepath,struct stat &statb)
{
    struct stat statbuf;
    if((stat(filepath,&statbuf)) != 0)
    {
        exit(-1);
    }

    if(statbuf.st_mtime > statb.st_mtime)
    {
        statb = statbuf;
        return 1;
    }

    return 0;
    
}

int main(int argc, char* argv[])
{
    struct stat statbuf;
    struct tm dm;
    time_t oldTime;
    FILE *f;
    char line[1024];
    std::queue<std::string> lines;

    if(argc != 2)
    {
        printf("Chybi cesta k souboru \n");
    }

    if((stat(argv[1],&statbuf)) == 0)
    {
        oldTime = statbuf.st_mtime;
        f = fopen(argv[1],"r");
        while(fgets(line, sizeof(line), f) != NULL)
        {
            lines.push(line);
        }
        fclose(f);
    }
    else
    {
        printf("Soubor nelze najit \n");
    }

    while(1)
    {
        if(file_is_modified(argv[1], statbuf) == 1)
        {
            f = fopen(argv[1],"r");
            while(fgets(line, sizeof(line), f) != NULL)
            {
                while(!lines.empty())
                {
                    if(lines.front() != line)
                    {
                        printf("%s", line);
                        lines.push(line);
                    }
                }
            }
            fclose(f);
        }

    }

}