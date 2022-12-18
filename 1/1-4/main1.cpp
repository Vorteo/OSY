#include <stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

#define N 10000

int main(int argc, char* argv[])
{
    int array[N];
    int min_array[20]; // array pro 20 nejmensich cisel 

    srand(time(NULL));
    for(int i = 0; i < N; i++)
    {
        array[i] = rand()%10000;
    }

    int fd[2];
    if(pipe(fd) == -1)
    {
        fprintf(stderr,"Vytvoreni pipe se neprovedlo \n");
        return -1;
    }

    int pid;

    int lower_index, upper_index; // dolni a horni indexy pro rozdeleni pole pro childy

    for(int i = 0; i < 20; i++)
    {
        pid = fork();

        if(pid == 0)
        {
            // child process
                     
            lower_index = i * 500;              // vypocet rozmezi pro vyber cisel 
            upper_index = (i + 1) * 500;

            int minimum = lower_index;      // index prvniho jako prvni minimum prvek

            for(int j = lower_index; j < upper_index; j++)      // prohledavani
            {
                if(array[j] < array[minimum])
                {
                    minimum = j;       
                }
            }

            write(fd[1], &array[minimum], sizeof(array[minimum]));
            _exit(0);
        }
        else
        {   
            read(fd[0], &min_array[i], sizeof(min_array[i])); 
        }
    }
    // presne netusim, kde se maji spravne zavirat pipe, protoze kdyz je zavru jeste v cyklu, 
    // tak uz k nim nebude mit nikdo pristup a provede se read a write jen jednoho cisla
    close(fd[0]);      
    close(fd[1]);

    int min = min_array[0];

    for(int i = 1; i < 20; i++)
    {
        if(min_array[i] < min)
        {
            min = min_array[i];
        }
    }
    printf("Nejmenší prvek v poli: %d \n", min);
}


