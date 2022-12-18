#include <stdio.h>

int main(int argc, char *argv[])
{
    FILE *f;
    char line[1024];

/*    if(argc < 2)
    {
        fprintf(stderr, "%s", "chybi parametr souboru");
    }
*/
    if(f = fopen(argv[1],"r"))
    {
        while(fgets(line, sizeof(line), f) != NULL)
        {
            printf("%s", line);
        }
        fclose(f);
    }
    else
    {
        fprintf(stderr, "%s", "Soubor neexistuje");
    }

   

    return 0;
}











