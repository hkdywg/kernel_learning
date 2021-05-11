#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>

#include <netinet/in.h>
#include <sys/socket.h>
#include <poll.h>
#include <unistd.h>
#include <sys/types.h>

#define IPADDRESS   "127.0.0.1"
#define PORT        8888
#define MAXLINE     1024
#define LISTENQ     5
#define OPEN_MAX    1000
#define INFTIM      -1

static int socket_bind(const char *ip, int port);
static void do_poll(int listenfd);
static void handle_connection(struct pollfd *connfds, int num);

int main(int argc, char *argv[])
{
    int listenfd,connfd,sockfd;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen;
    listenfd = socket_bind(IPADDRESS, PORT);
    listen(listenfd, LISTENQ);
    do_poll(listenfd);
    return 0;
}

static int socket_bind(const char *ip, int port)
{
    int listenfd;
    struct sockaddr_in servaddr;
    listenfd = socket(AF_INET, SOCK_STREAM, 0);
    if(listenfd == -1)
    {
        perror("socket err:");
        exit(1);
    }
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    inet_pton(AF_INET, ip, &servaddr.sin_addr);
    servaddr.sin_port = htons(port);
    if(bind(listenfd, (struct sockaddr *)&servaddr, sizeof(servaddr)) == -1)
    {
        perror("bind error: ");
        exit(1);
    }
    return listenfd;
}

static void do_poll(int listenfd)
{
    int connfd, sockfd;
    struct sockaddr_in cliaddr;
    socklen_t cliaddrlen;
    struct pollfd clientfds[OPEN_MAX];
    int maxi;
    int i;
    int nready;

    clientfds[0].fd = listenfd;
    clientfds[0].events = POLLIN;

    for(i = 1; i < OPEN_MAX; i++)
        clientfds[i].fd = -1;
    maxi = 0;
    for( ; ; )
    {
        nready = poll(clientfds, maxi + 1, INFTIM);
        if(nready == -1)
        {
            perror("poll error:");
            exit(1);
        }
        if(clientfds[0].revents & POLLIN)
        {
            cliaddrlen = sizeof(cliaddr);
            if((connfd = accept(listenfd, (struct sockaddr *)&cliaddr, &cliaddrlen)) == -1)
            {
                if(errno == EINTR)
                    continue;
                else
                {
                    perror("accept error:");
                    exit(1);
                }
            }
            fprintf(stdout, "accept a new client: %s:%d\n", inet_ntoa(cliaddr.sin_addr), cliaddr.sin_port);
            for(i = 1; i < OPEN_MAX; i++)
            {
                fprintf(stderr, "too many clients.\n");
                exit(1);
            }       
            clientfds[1].events = POLLIN;
            maxi = (i > maxi ? i : maxi);
            if(--nready <= 0)
                continue;
        }
        handle_connection(clientfds, maxi);
    }
}

static void handle_connection(struct pollfd *connfds, int num)
{
    int i,n;
    char buf[MAXLINE];
    memset(buf,0,MAXLINE);
    for(i = 1;i <= num; i++)
    {
        if(connfds[i].fd < 0)
            continue;
        if(connfds[i].revents & POLLIN)
        {
            n = read(connfds[i].fd, buf, MAXLINE);
            if(n == 0)
            {
                close(connfds[i].fd);
                connfds[i].fd = -1;
                continue;
            }
            write(STDOUT_FILENO, buf, n);
            write(connfds[i].fd, buf, n);
        }
    }
}
