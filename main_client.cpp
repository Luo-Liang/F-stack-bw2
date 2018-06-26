#include <stdio.h>
#include <sys/ioctl.h>
#include <stdlib.h>
#include <stdint.h>
#include <string.h>
#include <strings.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <errno.h>
#include <assert.h>
#include <string>

#include "ff_config.h"
#include "ff_api.h"
#include "ff_epoll.h"
#include "argparse.h"

#define MAX_EVENTS 512
#define CLIENT_PORT 1234
#define MAX_READ_SIZE 1024
struct epoll_event ev;
struct epoll_event events[MAX_EVENTS];

int epfd;
int sockfd;
int duration;
int interval;
timeval start, now, last;

int loop(void* arg)
{
    
}

int main(int argc, char *argv[])
{
    ff_init(argc, argv);
    //find --
    int skip = 0;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp("--", argv[i]) == 0)
        {
            skip = i + 1;
            break;
        }
    }

    ArgumentParser ap;
    ap.addArgument("--serverIp", 1, false);
    ap.addArgument("--duration", 1, true);
    ap.addArgument("--interval", 1, true);
    ap.addArgument("--pktSize", 1, true);
    argc -= skip;
    argv += skip;
    ap.ignoreFirstArgument(false);
    ap.parse(argc, (const char **)argv);

    duration = 10;
    if (ap.count("duration") > 0)
    {
        duration = atoi(ap.retrieve<std::string>("duration").c_str());
    }
    interval = 1;
    if (ap.count("interval") > 0)
    {
        interval = atoi(ap.retrieve<std::string>("interval").c_str());
    }
    sockfd = ff_socket(AF_INET, SOCK_STREAM, 0);
    printf("sockfd:%d\n", sockfd);
    if (sockfd < 0)
    {
        printf("ff_socket failed\n");
        exit(1);
    }

    int on = 1;
    ff_ioctl(sockfd, FIONBIO, &on);

    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(CLIENT_PORT);
    //bind to any of my address.
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    //my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = ff_bind(sockfd, (struct linux_sockaddr *)&my_addr, sizeof(my_addr));
    if (ret < 0)
    {
        printf("ff_bind failed\n");
        exit(1);
    }

    sockaddr_in remote_addr;
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(80);
    auto sip = ap.retrieve<std::string>("serverIp");
    inet_pton(AF_INET, sip.c_str(), &(remote_addr.sin_addr));
    ret = ff_connect(sockfd, (linux_sockaddr *)&remote_addr, sizeof(sockaddr_in));
    if (ret < 0 && errno != EINPROGRESS)
    {
        //ff_connect can return EINPROGRESS as it is always nb
        printf("ff_connect failed %d: %s\n", errno, strerror(errno));
        exit(1);
    }

    assert((epfd = ff_epoll_create(0)) > 0);
    ev.data.fd = sockfd;
    ev.events = EPOLLIN;
    ff_epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
    char req[] = "START";
    gettimeofday(&start, NULL);
    ff_write(sockfd, req, sizeof(req));
    size_t bytesRecv = 0;
    while (true)
    {
        gettimeofday(&now, NULL);
        if (now.tv_sec - start.tv_sec > duration)
        {
            break;
        }
        if (now.tv_sec - last.tv_sec >= interval)
        {
            //print current bandwidth.
            printf("BW = %fGbps\n", 8.0 * bytesRecv / interval / 1024 / 1024 / 1024);
        }
        int nevents = ff_epoll_wait(epfd, events, MAX_EVENTS, 0);
        int i;

        for (i = 0; i < nevents; ++i)
        {
            if (events[i].events & EPOLLERR)
            {
                /* Simply close socket */
                ff_epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                ff_close(events[i].data.fd);
                printf("EPOLLERR\n");
                exit(-1);
            }
            else if (events[i].events & EPOLLIN)
            {
                char buf[MAX_READ_SIZE];
                size_t readlen = ff_read(events[i].data.fd, buf, sizeof(buf));
                if (readlen > 0)
                {
                    bytesRecv += readlen;
                    ff_write(events[i].data.fd, req, sizeof(req));
                }
                else
                {
                    ff_epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    ff_close(events[i].data.fd);
                }
            }
            else
            {
                printf("unknown event: %8.8X\n", events[i].events);
            }
        }
        last = now;
    }
    return 0;
}
