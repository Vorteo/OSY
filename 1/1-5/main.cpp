#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

int main(int argc,char* argv[])
{
    int pid;  
    char buff[128];
    int fd[2];
    std::vector<char*> arguments;

    pipe(fd);

    for(int i = 1; i < argc; i++)
    {
        arguments.push_back(argv[i]);
    }
    arguments.push_back(nullptr);

    if((pid = fork()) == 0)
    {
        close(fd[0]);
        dup2(fd[1],1);
        close(fd[1]);

        execvp(argv[1], arguments.data());
        exit(0);

    }
    else
    {
        close(fd[1]);
        
        while(read(fd[0],&buff,sizeof(buff)) != 0)
	    {
		    fprintf(stdout,"%s",buff);
	    }
        close(fd[0]);
        printf("\n");

        wait(nullptr);

    }
}