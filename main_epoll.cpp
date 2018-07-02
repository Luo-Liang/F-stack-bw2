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
#include <signal.h>
#include <stdlib.h>
#include <stdio.h>
#include "PLinkNixEpollEAL.h"
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

    int nevents = PLinkEpollWait(epfd, events, MAX_EVENTS, 0);
    int i;

    for (i = 0; i < nevents; ++i)
    {
        /* Handle new connect */
        if (events[i].data.fd == sockfd)
        {
            while (1)
            {
                int nclientfd = PLinkAccept(sockfd, NULL, NULL);
                if (nclientfd < 0)
                {
                    break;
                }

                /* Add to event list */
                ev.data.fd = nclientfd;
                ev.events = EPOLLIN;
                if (PLinkEpollCtrl(epfd, EPOLL_CTL_ADD, nclientfd, &ev) != 0)
                {
                    printf("ff_epoll_ctl failed:%d, %s\n", errno,
                           strerror(errno));
                    break;
                }
                else
                {
                    printf("an incoming connection is accepted.\n");
                }
            }
        }
        else
        {
            if (events[i].events & EPOLLERR)
            {
                /* Simply close socket */
                PLinkEpollCtrl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                PLinkClose(events[i].data.fd);
            }
            else if (events[i].events & EPOLLIN)
            {
                char buf[256];
                size_t readlen = PLinkRead(events[i].data.fd, buf, sizeof(buf));
                if (readlen > 0)
                {
                    PLinkWrite(events[i].data.fd, contents.data(), contents.size());
                }
                else
                {
                    PLinkEpollCtrl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                    PLinkClose(events[i].data.fd);
                }
            }
            else
            {
                printf("unknown event: %8.8X\n", events[i].events);
            }
        }
    }
}

void ctrlHandler(sig_t s)
{
    printf("exiting... %d\n", s);
    PLinkClose(sockfd);
    exit(0);
}

int main(int argc, char *argv[])
{
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
    ap.addArgument("--pktSize", 1, true);
    int nargc = argc - skip;
    auto nargv = argv + skip;

    if (skip != 0)
    {
        ap.ignoreFirstArgument(false);
    }
    ap.parse(nargc, (const char **)nargv);

    PLinkInit(argc, argv);
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

    sockfd = PLinkSocket(AF_INET, SOCK_STREAM, 0);
    printf("sockfd:%d\n", sockfd);
    if (sockfd < 0)
    {
        printf("ff_socket failed\n");
        exit(1);
    }

    signal(SIGINT, ctrlHandler);
    //set up handler

    PLinkSetNonBlock(sockfd);

    struct sockaddr_in my_addr;
    bzero(&my_addr, sizeof(my_addr));
    my_addr.sin_family = AF_INET;
    my_addr.sin_port = htons(80);
    my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

    int ret = PLinkBind(sockfd, (const sockaddr *)&my_addr, sizeof(my_addr));
    if (ret < 0)
    {
        printf("ff_bind failed\n");
        exit(1);
    }

    ret = PLinkListen(sockfd, MAX_EVENTS);
    if (ret < 0)
    {
        printf("ff_listen failed\n");
        exit(1);
    }

    assert((epfd = PLinkEpollCreate()) > 0);
    ev.data.fd = sockfd;
    ev.events = EPOLLIN;
    PLinkEpollCtrl(epfd, EPOLL_CTL_ADD, sockfd, &ev);
    PLinkRun(loop, NULL);
    PLinkClose(sockfd);
    return 0;
}
