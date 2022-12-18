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


#define MQ_NAME "/mq_test"

#define MSG_SIZE 32

mqd_t g_msg_fd = -1;


static const char AlphaNumeric[] = "0123456789" 
                                    "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
                                    "abcdefghijklmnopqrstuvwxyz";

int MyLen = sizeof(AlphaNumeric) - 1;


static const char operations[] = "+-*/";

char randomStr()
{
    return AlphaNumeric[rand() % MyLen];
}

int main()
{
    g_msg_fd = mq_open(MQ_NAME, O_RDWR);

    if(g_msg_fd < 0)
    {
        printf("Nepovedlo se otevrit frontu zprav. Vytvarim novou.\n");

        mq_attr l_mqattr;
        bzero(&l_mqattr,sizeof(mq_attr));
        l_mqattr.mq_flags = 0;
        l_mqattr.mq_maxmsg = 20;
        l_mqattr.mq_msgsize = MSG_SIZE; 

        g_msg_fd = mq_open(MQ_NAME, O_RDWR | O_CREAT, 0660, &l_mqattr);
        if(g_msg_fd < 0)
        {
            printf("Nelze vytvorit fronta zprav!\n");
            exit(1);
        }
        else
        {
            printf("Fronta byla uspesne vytvorena.\n");
        }
    }
    else
    {
        printf("Fronta byla uspesne otevrena.\n");
    }

    srand(time(NULL));

    char buff[MSG_SIZE];

    while(1)
    {
        usleep(1000000);
        
        int first  = rand() % 100 + 1;
        int second = rand() % 100 + 1;
        char opt =  operations[rand() % (sizeof(operations)-1)];
        sprintf(buff,"%d %c %d", first, opt, second);
        printf("%s \n", buff);

        /*
        int rndlength = rand() % 31 + 1 ;
        for(int i = 0; i < rndlength; i++)
        {
            
            buff[i] = randomStr();
        }
        printf("%s \n", buff);
        */
        
        int ret = mq_send(g_msg_fd, buff, strlen(buff), 0);
        if(ret < 0)
        {
            printf("Zprava nebyla odeslana do fronty.\n");
        }
        
        printf("Zprava byla uspesne odeslana do fronty.\n");
        
        memset(buff, 0, sizeof(buff));
    }

    mq_close(g_msg_fd);
}