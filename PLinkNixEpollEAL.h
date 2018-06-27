#include "ff_config.h"
#include "ff_api.h"
#include "ff_epoll.h"
#include <unistd.h>

enum PLinkEpollAPI
{
    UseLinux,
    UseFStack
};

extern PLinkEpollAPI apiSwitch;

int PLinkSocket(int domain, int type, int protocol)
{
    switch (apiSwitch)
    {
    case UseLinux:
        return socket(domain, type, protocol);
    case UseFStack:
    default:
        return ff_socket(domain, type, protocol);
    }
}

int PLinkEpollCtrl(int epfd, int op, int fd, epoll_event *event)
{
    switch (apiSwitch)
    {
    case UseLinux:
        return epoll_ctl(epfd, op, fd, event);
    case UseFStack:
    default:
        return ff_epoll_ctl(epfd, op, fd, event);
    }
}

int PLinkClose(int fd)
{
    switch (apiSwitch)
    {
    case UseLinux:
        return close(fd);
    case UseFStack:
    default:
        return ff_close(fd);
    }
}

ssize_t PLinkWrite(int fd, const void *buf, size_t count)
{
    switch (apiSwitch)
    {
    case UseLinux:
        return write(fd, buf, count);
        break;
    case UseFStack:
    default:
        return ff_write(fd, buf, count);
    }
}

int PLinkInit(size_t argc, char *argv[])
{
    switch (apiSwitch)
    {
    case UseLinux:
        return 0; //write(fd, buf, count);
    case UseFStack:
    default:
        return ff_init(argc, argv);
    }
}

//cannot simulate
int PLinkSetNonBlock(int fd)
{
    int on = 1;
    switch (apiSwitch)
    {
    case UseLinux:
        return ioctl(fd, FIONBIO, &on);
    case UseFStack:
    default:
        return ff_ioctl(fd, FIONBIO, &on);
    }
}

int PLinkConnect(int sockfd, const sockaddr *addr, socklen_t addrlen)
{
    switch (apiSwitch)
    {
    case UseLinux:
        return connect(sockfd, addr, addrlen);
    case UseFStack:
    default:
        return ff_connect(sockfd, (const linux_sockaddr *)addr, addrlen);
    }
}

void PLinkRun(loop_func_t loop, void *arg)
{
    switch (apiSwitch)
    {
    case UseLinux:
        while (true)
        {
            loop(arg);
        }
        break;
    case UseFStack:
    default:
        ff_run(loop, arg);
        break;
    }
}

int PLinkAccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
    switch (apiSwitch)
    {
    case UseLinux:
        return accept(sockfd, addr, addrlen);
    case UseFStack:
    default:
        return ff_accept(sockfd, (linux_sockaddr *)addr, addrlen);
    }
}

int PLinkListen(int sockfd, int backlog)
{
    switch (apiSwitch)
    {
    case UseLinux:
        return listen(sockfd, backlog);
    case UseFStack:
    default:
        return ff_listen(sockfd, backlog);
    }
}

int PLinkEpollCreate(int size)
{
    switch (apiSwitch)
    {
    case UseLinux:
        return epoll_create(size);
    case UseFStack:
    default:
        return ff_epoll_create(size);
    }
}

int PLinkBind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
    switch (apiSwitch)
    {
    case UseLinux:
        return bind(sockfd, addr, addrlen);
    case UseFStack:
    default:
        return ff_bind(sockfd, (linux_sockaddr *)addr, addrlen);
    }
}

ssize_t PLinkRead(int fd, void *buf, size_t count)
{
    switch (apiSwitch)
    {
    case UseLinux:
        return read(fd, buf, count);
    case UseFStack:
    default:
        return ff_read(fd, buf, count);
    }
}

int PLinkEpollWait(int epfd, epoll_event *events, int maxevents, int timeout)
{
    switch (apiSwitch)
    {
    case UseLinux:
        return epoll_wait(epfd, events, maxevents, timeout);
    case UseFStack:
        return ff_epoll_wait(epfd, events, maxevents, timeout);
    }
}