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
#include <vector>

#include "ff_config.h"
#include "ff_api.h"
#include "ff_epoll.h"
#include "argparse.h"

#define MAX_EVENTS 512

struct epoll_event ev;
struct epoll_event events[MAX_EVENTS];

int epfd;
int sockfd;
int requestSize;

std::vector<char> contents;
const char PLINK[] = "PLINK";

int loop(void *arg)
{
    /* Wait for events to happen */

    int nevents = ff_epoll_wait(epfd, events, MAX_EVENTS, 0);
    int i;

    for (i = 0; i < nevents; ++i)
    {
        /* Handle new connect */
        if (events[i].data.fd == sockfd)
        {
            while (1)
            {
                int nclientfd = ff_accept(sockfd, NULL, NULL);
                if (nclientfd < 0)
                {
                    break;
                }

                /* Add to event list */
                ev.data.fd = nclientfd;
                ev.events = EPOLLIN;
                if (ff_epoll_ctl(epfd, EPOLL_CTL_ADD, nclientfd, &ev) != 0)
                {
                    printf("ff_epoll_ctl failed:%d, %s\n", errno,
                           strerror(errno));
                    break;
                }
            }
        }
        else
        {
            if (events[i].events & EPOLLERR)
            {
                /* Simply close socket */
                ff_epoll_ctl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                ff_close(events[i].data.fd);
            }
            else if (events[i].events & EPOLLIN)
            {
                char buf[256];
                size_t readlen = ff_read(events[i].data.fd, buf, sizeof(buf));
                if (readlen > 0)
                {
                    ff_write(events[i].data.fd, contents.data(), contents.size());
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
    }
}

int main(int argc, char *argv[])
{
    ff_init(argc, argv);

    int skip = 0;
    for (int i = 0; i < argc; i++)
    {
        if (strcmp("--", argv[i]) == 0)
        {
            skip == i + 1;
            break;
        }
    }

    argc -= skip;
    argv += skip;

    ArgumentParser ap;
    ap.addArgument("--pktSize", 1, false);
    ap.parse(argc, (const char **)argv);
    requestSize = 64;
    if (ap.count("pktSize") > 0)
    {
        requestSize = atoi(ap.retrieve<std::string>("pktSize").c_str());
    }

    contents.resize(requestSize);
    for (int i = 0; i < requestSize; i++)
    {
        contents.at(i) = PLINK[i % sizeof(PLINK)];
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
    my_addr.sin_port = htons(80);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = ff_bind(sockfd, (struct linux_sockaddr *)&my_addr, sizeof(my_addr));
    if (ret < 0)
    {
        printf("ff_bind failed\n");
        exit(1);
    }

    ret = ff_listen(sockfd, MAX_EVENTS);
    if (ret < 0)
    {
        printf("ff_listen failed\n");
        exit(1);
    }

    assert((epfd = ff_epoll_create(0)) > 0);
    ev.data.fd = sockfd;
    ev.events = EPOLLIN;
    ff_epoll_ctl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
    ff_run(loop, NULL);
    return 0;
}
