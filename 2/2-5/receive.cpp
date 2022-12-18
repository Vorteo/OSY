#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/types.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <time.h>
#include <string>


#define MQ_NAME "/mq_test"

#define MSG_SIZE 32

mqd_t g_msg_fd = -1;

int main()
{
    g_msg_fd = mq_open(MQ_NAME, O_RDWR);
    if(g_msg_fd < 0)
    {
        printf("Nejde otevrit frontu zprav.\n");
        exit(1);
    }
    else
    {
        printf("Fronta uspesne otevrena.\n");
    }

    char buff[MSG_SIZE];

    while(1)
    {
        int ret = mq_receive(g_msg_fd, buff, sizeof(buff), 0);
        if(ret > 0)
        {
            buff[ret] = '\0';
            printf("Prijata zprava od sendera: %s \n", buff);

         
            char* token = strtok(buff, " ");
            char* first = token;
            printf("Prvni: %s\n", token);
            token = strtok(NULL, " ");
            char* opt = token;
            printf("Operace: %s\n", opt);
            token = strtok(NULL, " ");
            char* second = token;
            printf("Druhy: %s\n", second);

            char* argv_list[] = {"expr", first, opt, second, NULL};

            char result[64];

            int fd[2];
            pipe(fd);

            if(fork() == 0)
            {
                close(fd[0]);
                dup2(fd[1], 1);
                close(fd[1]);

                execvp(argv_list[0], argv_list);
                exit(0);
            }
            else
            {

                close(fd[1]);
                int res = 0;
                while((res = read(fd[0],&result,sizeof(result))) != 0)
                {
                    printf("Pocet bajtu prijato: %d \n", res);
                    printf("Vysledek prikladu: %s \n", result);
                }             
                close(fd[0]);
            }

            usleep(2000000);
        }
        else 
        {
            printf("Dosla chyba pri prijimani zprav. \n");
            exit(1);
        }
    }
}