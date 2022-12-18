//***************************************************************************
//
// Program example for labs in subject Operating Systems
//
// Petr Olivka, Dept. of Computer Science, petr.olivka@vsb.cz, 2017
//
// Example of socket server.
//
// This program is example of socket server and it allows to connect and serve
// the only one client.
// The mandatory argument of program is port number for listening.
//
//***************************************************************************

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <stdarg.h>
#include <poll.h>
#include <sys/socket.h>
#include <sys/param.h>
#include <sys/time.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <errno.h>
#include <pthread.h>
#include <semaphore.h>

#define STR_CLOSE "close"
#define STR_QUIT "quit"

//***************************************************************************
// log messages

#define LOG_ERROR 0 // errors
#define LOG_INFO 1  // information and notifications
#define LOG_DEBUG 2 // debug messages

#define NUMTHREADS 3

typedef struct
{
    pthread_t thread_id;
    int l_sock_client;
    int index;
} Param;

typedef struct
{
    int fd;
    char mode;
} Client;

Client clients[NUMTHREADS];

int TCounter = 0; // actual thread count

int logoutId = -1;

sem_t wmutex;

// debug flag
int g_debug = LOG_INFO;

void log_msg(int t_log_level, const char *t_form, ...)
{
    const char *out_fmt[] = {
        "ERR: (%d-%s) %s\n",
        "INF: %s\n",
        "DEB: %s\n"};

    if (t_log_level && t_log_level > g_debug)
        return;

    char l_buf[1024];
    va_list l_arg;
    va_start(l_arg, t_form);
    vsprintf(l_buf, t_form, l_arg);
    va_end(l_arg);

    switch (t_log_level)
    {
    case LOG_INFO:
    case LOG_DEBUG:
        fprintf(stdout, out_fmt[t_log_level], l_buf);
        break;

    case LOG_ERROR:
        fprintf(stderr, out_fmt[t_log_level], errno, strerror(errno), l_buf);
        break;
    }
}

//***************************************************************************
// help

void help(int t_narg, char **t_args)
{
    if (t_narg <= 1 || !strcmp(t_args[1], "-h"))
    {
        printf(
            "\n"
            "  Socket server example.\n"
            "\n"
            "  Use: %s [-h -d] port_number\n"
            "\n"
            "    -d  debug mode \n"
            "    -h  this help\n"
            "\n",
            t_args[0]);

        exit(0);
    }

    if (!strcmp(t_args[1], "-d"))
        g_debug = LOG_DEBUG;
}

//***************************************************************************

void *manageClient(void *t_par)
{
    Param *lpar = (Param *)t_par;
    int l_sock_client = lpar->l_sock_client;
    int index = lpar->index;
    clients[index].fd = l_sock_client;

    while (1)
    { // communication
        char l_buf[256];

        // data from client?
        if (l_sock_client)
        {
            // read data from socket
            int l_len = read(l_sock_client, l_buf, sizeof(l_buf));
            l_buf[l_len] = '\0';

            if (!l_len)
            {
                log_msg(LOG_DEBUG, "Client closed socket!");
                close(l_sock_client);
                break;
            }
            else if (l_len < 0)
            {
                log_msg(LOG_DEBUG, "Unable to read data from client.");
            }
            else
            {
                log_msg(LOG_DEBUG, "Read %d bytes from client.", l_len);
            }

            if (clients[index].mode == ' ')
            {
                char mode;
                if (l_len == 2)
                {
                    mode = l_buf[0];
                    if (mode == 'w' || mode == 'r')
                    {
                        clients[index].mode = mode;
                    }
                }
            }

            // writer
            if (clients[index].mode == 'w')
            {
                if (strncasecmp(l_buf, "close", strlen(STR_CLOSE)))
                {
                    // Critical section start
                    sem_wait(&wmutex);

                    FILE *file = fopen("test.txt", "a");
                    if (file == NULL)
                    {
                        perror("Error opening file \n");
                        pthread_exit((void *)lpar->thread_id);
                    }

                    fprintf(file, "%s", l_buf);
                    write(l_sock_client, l_buf, l_len);
                    fclose(file);

                    // Critical section end
                    sem_post(&wmutex);
                }
            }

            // reader
            else if (clients[index].mode == 'r')
            {   
                //fpos_t position;
                int r = 0;

                FILE *file = fopen("test.txt", "r");
                if (file == NULL)
                {
                    perror("Error openning file \n");
                    exit(1);
                }

                while (1)
                {
                    if (r == 0)
                    {
                        char c;
                        char start[21] = "--Zacatek souboru--\n";
                        char end[19] = "--Konec souboru--\n";
                        write(l_sock_client, start, sizeof(start));

                        while ((c = fgetc(file)) != EOF)
                        {
                            write(l_sock_client, &c, sizeof(char));
                        }
                        fseek(file, 0, SEEK_END);
                       // fgetpos(,&position);
                        write(l_sock_client, end, sizeof(end));

                        r++;
                    }
                    else
                    {
                        char c;
                        while((c = fgetc(file)) != EOF)
                        {
                            write(l_sock_client, &c, sizeof(char));
                        }
                    }
                    //read(l_sock_client, l_buf, sizeof(l_buf));
                }
                fclose(file);
            }

            // close request?
            if (!strncasecmp(l_buf, "close", strlen(STR_CLOSE)))
            {
                TCounter--;
                clients[index].fd = -1;
                clients[index].mode = ' ';
                logoutId = index;

                log_msg(LOG_INFO, "Client sent 'close' request to close connection.");
                close(l_sock_client);
                log_msg(LOG_INFO, "Connection closed. Waiting for new client.");

                pthread_exit((void *)lpar->thread_id);
                break;
            }
        }
    } // while communication
}


int main(int t_narg, char **t_args)
{
    if (t_narg <= 1)
        help(t_narg, t_args);

    int l_port = 0;

    if (sem_init(&wmutex, 0, 1))
    {
        perror("Error while initializing semaphor mutex\n");
        exit(1);
    }

    pthread_t thread_id[NUMTHREADS]; // threads

    for (int i = 0; i < NUMTHREADS; i++)
    {
        clients[i].fd = -1;
        clients[i].mode = ' ';
    }

    // parsing arguments
    for (int i = 1; i < t_narg; i++)
    {
        if (!strcmp(t_args[i], "-d"))
            g_debug = LOG_DEBUG;

        if (!strcmp(t_args[i], "-h"))
            help(t_narg, t_args);

        if (*t_args[i] != '-' && !l_port)
        {
            l_port = atoi(t_args[i]);
            break;
        }
    }

    if (l_port <= 0)
    {
        log_msg(LOG_INFO, "Bad or missing port number %d!", l_port);
        help(t_narg, t_args);
    }

    log_msg(LOG_INFO, "Server will listen on port: %d.", l_port);

    // socket creation
    int l_sock_listen = socket(AF_INET, SOCK_STREAM, 0);
    if (l_sock_listen == -1)
    {
        log_msg(LOG_ERROR, "Unable to create socket.");
        exit(1);
    }

    in_addr l_addr_any = {INADDR_ANY};
    sockaddr_in l_srv_addr;
    l_srv_addr.sin_family = AF_INET;
    l_srv_addr.sin_port = htons(l_port);
    l_srv_addr.sin_addr = l_addr_any;

    // Enable the port number reusing
    int l_opt = 1;
    if (setsockopt(l_sock_listen, SOL_SOCKET, SO_REUSEADDR, &l_opt, sizeof(l_opt)) < 0)
        log_msg(LOG_ERROR, "Unable to set socket option!");

    // assign port number to socket
    if (bind(l_sock_listen, (const sockaddr *)&l_srv_addr, sizeof(l_srv_addr)) < 0)
    {
        log_msg(LOG_ERROR, "Bind failed!");
        close(l_sock_listen);
        exit(1);
    }

    // listenig on set port
    if (listen(l_sock_listen, 1) < 0)
    {
        log_msg(LOG_ERROR, "Unable to listen on given port!");
        close(l_sock_listen);
        exit(1);
    }

    log_msg(LOG_INFO, "Enter 'quit' to quit server.");

    // go!
    while (1)
    {
        int l_sock_client = -1;
        while (1) // wait for new client
        {
            /* tahle cast blokuje pripojeni klientu
            char buff[64];
            read(STDIN_FILENO, buff, sizeof(buff));
            if (!strncmp(buff, STR_QUIT, strlen(STR_QUIT)))
            {
                log_msg(LOG_INFO, "Request to 'quit' entered.");
                close(l_sock_listen);
                exit(0);
            }
            */

            if (l_sock_client)
            { // new client?
                sockaddr_in l_rsa;
                int l_rsa_size = sizeof(l_rsa);
                // new connection
                l_sock_client = accept(l_sock_listen, (sockaddr *)&l_rsa, (socklen_t *)&l_rsa_size);
                if (l_sock_client == -1)
                {
                    log_msg(LOG_ERROR, "Unable to accept new client.");
                    close(l_sock_listen);
                    exit(1);
                }

                // only 3 clients can connect to server
                if (TCounter >= NUMTHREADS)
                {
                    const char *message = "Unable to connect more clients";
                    write(l_sock_client, message, strlen(message));
                    write(l_sock_client, "\n", sizeof(char));
                    close(l_sock_client);
                    continue;
                }

                uint l_lsa = sizeof(l_srv_addr);
                // my IP
                getsockname(l_sock_client, (sockaddr *)&l_srv_addr, &l_lsa);
                log_msg(LOG_INFO, "My IP: '%s'  port: %d",
                        inet_ntoa(l_srv_addr.sin_addr), ntohs(l_srv_addr.sin_port));
                // client IP
                getpeername(l_sock_client, (sockaddr *)&l_srv_addr, &l_lsa);
                log_msg(LOG_INFO, "Client IP: '%s'  port: %d",
                        inet_ntoa(l_srv_addr.sin_addr), ntohs(l_srv_addr.sin_port));

                int index = 0;
                // check free client
                if (logoutId >= 0)
                {
                    index == logoutId;
                }
                else
                {
                    index = TCounter;
                }

                Param *thread_param = new Param;
                thread_param->l_sock_client = l_sock_client;
                thread_param->thread_id = thread_id[index];
                thread_param->index = index;

                // thread creation
                int err = pthread_create(&thread_id[TCounter], nullptr, manageClient, (void *)thread_param);
                if (err)
                {
                    log_msg(LOG_INFO, "Unable to create thread %d.", index);
                }
                else
                {
                    log_msg(LOG_DEBUG, "Thread %d created - system id 0x%X.", index, thread_id[index]);
                    TCounter++;
                }
            }

        } // while wait for client

    } // while ( 1 )

    for (int i = 0; i < NUMTHREADS; i++)
    {
        pthread_join(thread_id[i], nullptr);
    }

    sem_destroy(&wmutex);

    return 0;
}