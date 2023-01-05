// Server cte z klienta po znacich
// klient si vytvori proces a v nem otevre soubor, cte a posila do serveru. Nasledne konci, konec souboru je jako \n\n\n
// server prida ke kazdemu radku jeho cislo
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
#include <sys/stat.h>

#define STR_CLOSE "close"
#define STR_QUIT "quit"

#define NUMTHREADS 5
#define SEM_MUTEX_NAME "/sem_mutex"
sem_t *g_sem_mutex = nullptr;

//***************************************************************************
// log messages

#define LOG_ERROR 0 // errors
#define LOG_INFO 1  // information and notifications
#define LOG_DEBUG 2 // debug messages

// debug flag
int g_debug = LOG_INFO;

struct Param
{
    int l_socket_client;
    int l_sock_listen;
    pthread_t thread_id;
    int index;
};

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

void *manageClient(void *t_par)
{
    Param *param = (Param *)t_par;
    int l_sock_client = param->l_socket_client;
    int index = param->index;
    pthread_t thread_id = param->thread_id;

    pollfd l_read_poll[2];
    l_read_poll[0].fd = STDIN_FILENO;
    l_read_poll[0].events = POLLIN;
    l_read_poll[1].fd = l_sock_client;
    l_read_poll[1].events = POLLIN;

    while (1)
    { // communication

        // select from fds
        int l_poll = poll(l_read_poll, 2, -1);
        if (l_poll < 0)
        {
            log_msg(LOG_ERROR, "Function poll failed!");
            exit(1);
        }

        // data from client?
        if (l_read_poll[1].revents & POLLIN)
        {
            sem_wait(g_sem_mutex);
            int n = 0;
            int endCounter = 0;
            while (1)
            {
                char buff[64];
                if (n == 0)
                {
                    sprintf(buff, "radek %d - ", n);
                    write(l_sock_client, buff, sizeof(buff));
                    n++;
                }

                char c;
                int len = read(l_sock_client, &c, sizeof(char));
                if(!len)
                {
                    break;
                }

                if ( endCounter >= 1 && c != '\n')
                {
                    endCounter = 0;
                    
                }               
                write(l_sock_client, &c, sizeof(char));

                if (c == '\n')
                {
                    endCounter++;

                    if(endCounter == 3)
                    {
                        break;
                    }

                    sprintf(buff, "radek %d - ", n);
                    write(l_sock_client, buff, sizeof(buff));
                    n++;
                }
                usleep(500);
            }
            sem_post(g_sem_mutex);
            break;
        }
    } // while communication

    close(l_sock_client);
    pthread_exit((void *)thread_id);
}

//***************************************************************************

int main(int t_narg, char **t_args)
{
    int TCounter = 0;
    pthread_t thread_id[NUMTHREADS];
    void *thread_status[NUMTHREADS];

    sem_unlink(SEM_MUTEX_NAME);
    g_sem_mutex = sem_open(SEM_MUTEX_NAME, O_RDWR);
    if (!g_sem_mutex)
    {
        log_msg(LOG_ERROR, "Unable to open semaphore. Create new one.");
        g_sem_mutex = sem_open(SEM_MUTEX_NAME, O_RDWR | O_CREAT, 0660, 1);
        if (!g_sem_mutex)
        {
            log_msg(LOG_ERROR, "Unable to create semaphore!");
            return 1;
        }

        log_msg(LOG_INFO, "Semaphore created.");
    }

    if (t_narg <= 1)
        help(t_narg, t_args);

    int l_port = 0;

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

        // list of fd sources
        pollfd l_read_poll[2];

        l_read_poll[0].fd = STDIN_FILENO;
        l_read_poll[0].events = POLLIN;
        l_read_poll[1].fd = l_sock_listen;
        l_read_poll[1].events = POLLIN;

        while (1) // wait for new client
        {
            // select from fds
            int l_poll = poll(l_read_poll, 2, -1);

            if (l_poll < 0)
            {
                log_msg(LOG_ERROR, "Function poll failed!");
                exit(1);
            }

            if (l_read_poll[0].revents & POLLIN)
            { // data on stdin
                char buf[128];
                int len = read(STDIN_FILENO, buf, sizeof(buf));
                if (len < 0)
                {
                    log_msg(LOG_DEBUG, "Unable to read from stdin!");
                    exit(1);
                }

                log_msg(LOG_DEBUG, "Read %d bytes from stdin");
                // request to quit?
                if (!strncmp(buf, STR_QUIT, strlen(STR_QUIT)))
                {
                    log_msg(LOG_INFO, "Request to 'quit' entered.");
                    close(l_sock_listen);
                    exit(0);
                }
            }

            if (l_read_poll[1].revents & POLLIN)
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
                //
                if (TCounter >= NUMTHREADS)
                {
                    const char *message = "Neni mozne pripojit vice klientu";
                    write(l_sock_client, message, strlen(message));
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

                Param *thread_param = new Param;
                thread_param->l_socket_client = l_sock_client;
                thread_param->thread_id = thread_id[TCounter];
                thread_param->index = TCounter;
                thread_param->l_sock_listen = l_sock_listen;

                int err = pthread_create(&thread_id[TCounter], nullptr, manageClient, (void *)thread_param);
                if (err)
                {
                    log_msg(LOG_INFO, "Nepodarilo se vytvorit vlakno %d.", TCounter);
                }
                else
                {
                    log_msg(LOG_INFO, "Vytvorilo se vlakno %d.", TCounter);
                    TCounter++;
                }
            }
            /*
            for (int i = 0; i < NUMTHREADS; i++)
            {
                pthread_join(thread_id[i], &thread_status[i]);
            }
            */
        } // while wait for client
    }     // while ( 1 )

    sem_destroy(g_sem_mutex);
    return 0;
}


