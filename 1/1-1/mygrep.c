#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>

int main(int argc, char *argv[])
{
    char line[1024];
    int counter = 0;
    int opt;

    while((opt = getopt(argc, argv, "c")) != -1)
    {
        switch (opt)
        {
        case 'c':
            while(fgets(line, sizeof(line), stdin) != NULL)
            {
            if(strstr(line, argv[optind]) != NULL)
            {
                counter++;
            }
            }
            break;
        
        default:
            fprintf(stderr, "%s", "Chybny parametr");
            exit(EXIT_FAILURE);
        }
    }
       
    printf(" Pocet vyskytu: %d \n", counter);
   
    return 0;
}