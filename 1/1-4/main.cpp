#include <stdio.h>
#include<stdlib.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <fcntl.h>
#include <time.h>

int main(int argc, char* argv[])
{
    int fd[2];
    char buf[100];
    pid_t p1, p2;

    if(pipe(fd) == -1)
    {
        fprintf(stderr,"Vytvoreni pipe se neprovedlo \n");
        return -1;
    }
    else
    {
        printf("Pipe uspesne vytvorena\n");
    }

    p1 = fork();

    if(p1 == 0)
    {
        // first child process
        printf("Spouští se první child proces\n");
        close(fd[0]);      
        srand(time(NULL) ^ (getpid()<<16));
        int time = rand() % (10 - 5 + 1) + 5;
        printf("time sleep: %d \n", time);
        sleep(time);
        sprintf(buf,"Child process pid: %d and parent id: %d \n",getpid(), getppid());
        write(fd[1],buf, sizeof(buf));
        close(fd[1]);
    }
    else
    {
        p2 = fork();

        if(p2 == 0)
        {
            printf("Spouští se druhý child proces \n");
            // second child process
            close(fd[0]);
            srand(time(NULL) ^ (getpid()<<16));
            int time = rand() % (10 - 5 + 1) + 5;
            printf("time sleep: %d \n", time);
            sleep(time);
            sprintf(buf,"Child process pid: %d and parent id: %d \n",getpid(), getppid());
            write(fd[1],buf, sizeof(buf));
            close(fd[1]);
        }
        else if(p2 > 0)
        {
            // parent process
            close(fd[1]);
            printf(" Vstuju se do Parent procesu \n");

           while(read(fd[0],&buf, sizeof(buf)) > 0)
           {
                printf("%s",buf);
                
           }
           printf("\n");
           close(fd[0]);
           wait(NULL);
        }
    }


}