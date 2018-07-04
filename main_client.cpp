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

#include "PLinkNixEpollEAL.h"
#include "argparse.h"

#define MAX_EVENTS 512
#define MAX_READ_SIZE 1024 * 1024 * 16 * 16
struct epoll_event events[MAX_EVENTS];

int epfd;
std::vector<int> sockfdVec;
int duration;
int interval;
timeval start, now, last;
size_t bytesRecv = 0;
const char req[] = "PLINK";
int allTransferInitiated = false;
std::string sip;
char buf[MAX_READ_SIZE];

int loop(void *arg)
{
    gettimeofday(&now, NULL);
    if (now.tv_sec - start.tv_sec > duration && allTransferInitiated == sockfdVec.size())
    {
        for (auto sockfd : sockfdVec)
        {
            PLinkClose(sockfd);
        }
        PLinkClose(epfd);
        exit(0);
    }
    if (now.tv_sec - last.tv_sec >= interval && allTransferInitiated == sockfdVec.size())
    {
        //print current bandwidth.
        printf("BW = %fGbps\n", 8.0 * bytesRecv / interval / 1024 / 1024 / 1024);
        bytesRecv = 0;
    }
    int nevents = PLinkEpollWait(epfd, events, MAX_EVENTS, 0);
    int i;

    for (i = 0; i < nevents; ++i)
    {
        if (events[i].events & EPOLLERR)
        {
            /* Simply close socket */
            PLinkEpollCtrl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
            PLinkClose(events[i].data.fd);
            printf("EPOLLERR. %s=?\n", strerror(errno));
            exit(-1);
        }
        if (events[i].events & EPOLLIN)
        {
            ssize_t readlen = PLinkRead(events[i].data.fd, buf, sizeof(buf));
            if (readlen > 0)
            {
                bytesRecv += readlen;
                PLinkWrite(events[i].data.fd, req, sizeof(req));
            }
            else
            {
                PLinkEpollCtrl(epfd, EPOLL_CTL_DEL, events[i].data.fd, NULL);
                PLinkClose(events[i].data.fd);
            }
        }
        if (events[i].events & EPOLLOUT)
        {
            allTransferInitiated++;
            events[i].events = EPOLLIN;
            PLinkEpollCtrl(epfd, EPOLL_CTL_MOD, events[i].data.fd, &events[i]);
            if (allTransferInitiated == sockfdVec.size())
            {
                gettimeofday(&start, NULL);
                for (auto sockfd : sockfdVec)
                {
                    auto sz = PLinkWrite(sockfd, req, sizeof(req));
                    //turn off epollout
                    printf("client initiating transfer of %lu bytes\n", sz);
                }
            }
        }
    }
    last = now;
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
    ap.addArgument("--serverIp", 1, false);
    ap.addArgument("--duration", 1, true);
    ap.addArgument("--interval", 1, true);
    ap.addArgument("--streams", 1, true);

    int nargc = argc - skip;
    auto nargv = argv + skip;
    if (skip != 0)
    {
        ap.ignoreFirstArgument(false);
    }
    ap.parse(nargc, (const char **)nargv);

    PLinkInit(argc, argv);
    //find --
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
    int sockCnt = 1;
    if (ap.count("streams") > 0)
    {
        sockCnt = atoi(ap.retrieve<std::string>("streams").c_str());
    }

    assert((epfd = PLinkEpollCreate()) > 0);
    srand((unsigned)time(0));
    auto CLIENT_PORT = (uint16_t)(1024 + rand() % (65536 - 1024));
    for (int i = 0; i < sockCnt; i++)
    {
        int currSockfd = PLinkSocket(AF_INET, SOCK_STREAM, 0);
        printf("sockfd:%d is initialized. will bind to port %d\n", currSockfd, CLIENT_PORT + i);
        if (currSockfd < 0)
        {
            printf("ff_socket failed\n");
            exit(1);
        }
        PLinkSetNonBlock(currSockfd);
        sockfdVec.push_back(currSockfd);
        struct sockaddr_in my_addr;
        bzero(&my_addr, sizeof(my_addr));
        my_addr.sin_family = AF_INET;
        my_addr.sin_port = htons(CLIENT_PORT + i);
        //bind to any of my address.
        my_addr.sin_addr.s_addr = htonl(INADDR_ANY);
        sip = ap.retrieve<std::string>("serverIp");
        //my_addr.sin_addr.s_addr = htonl(INADDR_ANY);

        int ret = PLinkBind(currSockfd, (const sockaddr *)&my_addr, sizeof(my_addr));
        if (ret < 0)
        {
            printf("PLinkBind failed, errno = %s\n", strerror(errno));
            for (auto sock : sockfdVec)
            {
                PLinkClose(sock);
            }
            exit(1);
        }
        struct epoll_event ev;
        ev.data.fd = currSockfd;
        ev.events = EPOLLIN | EPOLLOUT;
        PLinkEpollCtrl(epfd, EPOLL_CTL_ADD, currSockfd, &ev);
        sockaddr_in remote_addr;
        remote_addr.sin_family = AF_INET;
        remote_addr.sin_port = htons(80);
        inet_pton(AF_INET, sip.c_str(), &(remote_addr.sin_addr));
        ret = PLinkConnect(currSockfd, (const sockaddr *)&remote_addr, sizeof(sockaddr_in));
        if (ret < 0 && errno != EINPROGRESS)
        {
            //ff_connect can return EINPROGRESS as it is always nb
            printf("ff_connect failed %d: %s\n", errno, strerror(errno));
            exit(1);
        }
    }

    //printf("client initated transfer of %d bytes. err = %s?\n", sz, strerror(errno));

    PLinkRun(loop, NULL);
    return 0;
}
