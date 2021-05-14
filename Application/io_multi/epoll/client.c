#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/epoll.h>
#include <sys/types.h>
#include <arpa/inet.h>

#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <unistd.h>
#include <string.h>

#define MAXSIZE         1204
#define IPADDRESS       "127.0.0.1"
#define SERV_PORT       8787
#define FDSIZE          1024
#define EPOLLEVENTS     20

static void handle_connection(int sockfd);
static void handle_events(int epollfd, struct epoll_event *events, int num, int sockfd, char *buf);
static void do_read(int epollfd, int fd, int sockfd, char *buf);
static void do_write(int epollfd, int fd, int sockfd, char *buf);
static void add_event(int epollfd, int fd, int state);
static void delete_event(int epollfd, int fd, int state);

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in servaddr;
    socklen_t servaddrlen = sizeof(servaddr);
    int ret;

    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, IPADDRESS, &servaddr.sin_addr);
    ret = connect(sockfd, (struct sockaddr*)&servaddr, servaddrlen);
    if(ret == -1)
        return -1;
    handle_connection(sockfd);
    close(sockfd);
    return 0;
}

static void handle_connection(int sockfd)
{
    int epollfd;
    struct epoll_event events[EPOLLEVENTS];
    char buf[MAXSIZE];
    int ret;
    epollfd = epoll_create(FDSIZE);
    add_event(epollfd, STDIN_FILENO, EPOLLIN);
    add_event(epollfd, sockfd, EPOLLIN | EPOLLOUT);
    for( ; ; )
    {
        ret = epoll_wait(epollfd, events, EPOLLEVENTS, -1);
        if(ret == -1)
        {
            perror("epoll_waite err:");
            break;
        }
        handle_events(epollfd, events, ret, sockfd, buf);
        memset(buf, 0, MAXSIZE);
    }
    close(epollfd);
}

static void handle_events(int epollfd, struct epoll_event *events, int num, int sockfd, char *buf)
{
    int fd;
    int i;
    for(i = 0; i < num; i++)
    {
        fd = events[i].data.fd;
        if(events[i].events & EPOLLIN)
        {
            do_read(epollfd, fd, sockfd, buf);
        }
    }
}

static void do_read(int epollfd, int fd, int sockfd, char *buf)
{
    int nread;
    nread = read(fd, buf, MAXSIZE);
    if(fd != STDIN_FILENO)
    {
        printf("receive message : %s\n",buf);
        return;
    }
    if(nread == -1)
    {
        perror("read error:");
        close(fd);
    }
    else if(nread == 0)
    {
        fprintf(stderr, "server close.\n");
        close(fd);
    }
    else
    {
        write(sockfd, buf, strlen(buf));    
        memset(buf, 0, strlen(buf));
    }
}

static void add_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_ADD, fd, &ev);
}

static void delete_event(int epollfd, int fd, int state)
{
    struct epoll_event ev;
    ev.events = state;
    ev.data.fd = fd;
    epoll_ctl(epollfd, EPOLL_CTL_DEL, fd, &ev);
}
