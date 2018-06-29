#pragma once
#include "ff_config.h"
#include "ff_api.h"
#include "ff_epoll.h"
extern "C"
{
#include "anssock_intf.h"
#include "ans_errno.h"
}

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/times.h>

#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <netinet/in.h>
#include <termios.h>
#include <sys/epoll.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#ifndef __linux__
#ifdef __FreeBSD__
#include <sys/socket.h>
#else
#include <net/socket.h>
#endif
#endif

#include <sys/time.h>

enum PLinkEpollAPI
{
    UseLinux,
    UseANS,
    UseFStack
};

extern PLinkEpollAPI apiSwitch;
extern "C" int anssock_socket(int domain, int type, int protocol);

int PLinkSocket(int domain, int type, int protocol)
{
    switch (apiSwitch)
    {
    case UseLinux:
        return socket(domain, type, protocol);
    case UseANS:
        return anssock_socket(domain, type, protocol);
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
    case UseANS:
        return anssock_epoll_ctl(epfd, op, fd, event);
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
    case UseANS:
        return anssock_close(fd);
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
    case UseANS:
        return anssock_write(fd, buf, count);
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
    case UseANS:
        return anssock_init(NULL);
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
    case UseANS:
        return 0;
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
    case UseANS:
        return anssock_connect(sockfd, addr, addrlen);
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
    case UseANS:
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
    case UseANS:
        return anssock_accept(sockfd, addr, addrlen);
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
    case UseANS:
        return anssock_listen(sockfd, backlog);
    case UseFStack:
    default:
        return ff_listen(sockfd, backlog);
    }
}

int PLinkEpollCreate()
{
    switch (apiSwitch)
    {
    case UseLinux:
        return epoll_create(1);
    case UseANS:
        return anssock_epoll_create(512);
    case UseFStack:
    default:
        return ff_epoll_create(1);
    }
}

int PLinkBind(int sockfd, const sockaddr *addr, socklen_t addrlen)
{
    switch (apiSwitch)
    {
    case UseLinux:
        return bind(sockfd, addr, addrlen);
    case UseANS:
        return anssock_bind(sockfd, addr, addrlen);
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
    case UseANS:
        return anssock_read(fd, buf, count);
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
    case UseANS:
        return anssock_epoll_wait(epfd, events, maxevents, timeout);
    case UseFStack:
        return ff_epoll_wait(epfd, events, maxevents, timeout);
    }
}