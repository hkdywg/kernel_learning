#include <netinet/in.h>
#include <sys/socket.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <poll.h>
#include <time.h>
#include <unistd.h>
#include <sys/types.h>

#define MAXLINE     1024
#define IPADDRESS   "127.0.0.1"
#define SERV_PORT   8888

#define max(a,b) (a > b) ? a : b

static void handle_connection(int sockfd);

int main(int argc, char *argv[])
{
    int sockfd;
    struct sockaddr_in servaddr;
    sockfd = socket(AF_INET, SOCK_STREAM, 0);
    bzero(&servaddr, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_port = htons(SERV_PORT);
    inet_pton(AF_INET, IPADDRESS, &servaddr.sin_addr);
    connect(sockfd, (struct sockaddr *)&servaddr, sizeof(servaddr));
    handle_connection(sockfd);
    return 0;
}

static void handle_connection(int sockfd)
{
    char sendline[MAXLINE],recvline[MAXLINE];
    int maxfdp, stdineof;
    struct pollfd pfds[2];
    int n;
    
    pfds[0].fd = sockfd;
    pfds[0].events = POLLIN;
    for( ; ; )
    {
        poll(pfds, 2, -1);
        if(pfds[0].revents & POLLIN)
        {
            n = read(sockfd, recvline, MAXLINE);
            if(n == 0)
            {
                fprintf(stderr, "client: server is closed.\n");
                close(sockfd);
            }
            write(STDOUT_FILENO, recvline, n);
        }
        if(pfds[1].revents & POLLIN)
        {
            n = read(STDIN_FILENO, sendline, MAXLINE);
            if(n == 0)
            {
                shutdown(sockfd, SHUT_WR);
                continue;
            }
            write(sockfd, sendline, n);
        }
    }
}
