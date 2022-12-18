#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <vector>

int main(int argc, char* argv[])
{
   int pid, pid1;
   char buff[128];
   int fd[2];
   std::vector<char*> arguments;
    
   for(int i = 2; i < argc; i++)
   {
        arguments.push_back(argv[i]);
   }
   arguments.push_back(nullptr);

   pipe(fd);

   if((pid = fork()) == 0)
   {  
        close(fd[0]);

        dup2(fd[1], 1);
        close(fd[1]);
        
        execvp(arguments[0],arguments.data());
        exit(0);
   }
   else
   {
        if((pid1 = fork()) == 0)
        {           
            close(fd[1]);

            FILE* f = fopen(argv[1],"w");

            int l = 0;            
            while((l = read(fd[0], buff, sizeof(buff))) > 0)
            {
                //write(1,buff,l);
                fwrite(buff, 1, l, f);
            }
            
            close(fd[0]); 
            fclose(f);  
            exit(0);
        }
        
        else
        {
            close(fd[0]);
            close(fd[1]);

            while(wait(nullptr) > 0);
        }   
   } 
}