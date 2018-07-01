#pragma once
#define LINUX 0
#define ANS 1
#define FSTACK 2

#if API == ANS
extern "C"
{
#include "anssock_intf.h"
#include "ans_errno.h"
}
#elif API == FSTACK
#include "ff_config.h"
#include "ff_api.h"
#include "ff_epoll.h"
#elif API == LINUX
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#else
#error "Specify API"
#endif

#include <unistd.h>
#include <stdio.h>
#include <stdlib.h>
#include <sched.h>
#include <sys/times.h>
#include <stdint.h>
#include <string.h>
#include <stdarg.h>
#include <errno.h>
#include <termios.h>
#include <sys/types.h>
#include <sys/time.h>
#include <sys/epoll.h>

typedef int (*PLinkLoopFunction)(void *arg);


int PLinkSocket(int domain, int type, int protocol)
{
#if API == LINUX
    return socket(domain, type, protocol);
#elif API == ANS
    return anssock_socket(domain, type, protocol);
#elif API == FSTACK
    return ff_socket(domain, type, protocol);
#else
#error "Specify API"
#endif
}

int PLinkEpollCtrl(int epfd, int op, int fd, epoll_event *event)
{
#if API == LINUX
    return epoll_ctl(epfd, op, fd, event);
#elif API == ANS
    return anssock_epoll_ctl(epfd, op, fd, event);
#elif API == FSTACK
    return ff_epoll_ctl(epfd, op, fd, event);
#else
#error "Specify API"
#endif
}

int PLinkClose(int fd)
{
#if API == LINUX
    return close(fd);
#elif API == ANS
    return anssock_close(fd);
#elif API == FSTACK
    return ff_close(fd);
#else
#error "Specify API"
#endif
}

ssize_t PLinkWrite(int fd, const void *buf, size_t count)
{
#if API == LINUX
    return write(fd, buf, count);
#elif API == ANS
    return anssock_write(fd, buf, count);
#elif API == FSTACK
    return ff_write(fd, buf, count);
#else
#error "Specify API"
#endif
}

int PLinkInit(size_t argc, char *argv[])
{
#if API == LINUX
    return 0; //write(fd, buf, count);
#elif API == ANS
    return anssock_init(NULL);
#elif API == FSTACK
    return ff_init(argc, argv);
#else
#error "Specify API"
#endif
}

//cannot simulate
int PLinkSetNonBlock(int fd)
{
    int on = 1;
#if API == LINUX
    return ioctl(fd, FIONBIO, &on);
#elif API == ANS
    return 0;
#elif API == FSTACK
    return ff_ioctl(fd, FIONBIO, &on);
#else
#error "Specify API"
#endif
}

int PLinkConnect(int sockfd, const sockaddr *addr, socklen_t addrlen)
{
#if API == LINUX
    return connect(sockfd, addr, addrlen);
#elif API == ANS
    return anssock_connect(sockfd, addr, addrlen);
#elif API == FSTACK
    return ff_connect(sockfd, (const linux_sockaddr *)addr, addrlen);
#else
#error "Specify API"
#endif
}

void PLinkRun(PLinkLoopFunction loop, void *arg)
{
#if API == LINUX || API == ANS
    while (true)
    {
        loop(arg);
    }
#elif API == FSTACK
    ff_run(loop, arg);
#else
#error "Specify API"
#endif
}

int PLinkAccept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
#if API == LINUX
    return accept(sockfd, addr, addrlen);
#elif API == ANS
    return anssock_accept(sockfd, addr, addrlen);
#elif API == FSTACK
    return ff_accept(sockfd, (linux_sockaddr *)addr, addrlen);
#else
#error "Specify API"
#endif
}

int PLinkListen(int sockfd, int backlog)
{
#if API == LINUX
    return listen(sockfd, backlog);
#elif API == ANS
    return anssock_listen(sockfd, backlog);
#elif API == FSTACK
    return ff_listen(sockfd, backlog);
#else
#error "Specify API"
#endif
}

int PLinkEpollCreate()
{
#if API == LINUX
    return epoll_create(1);
#elif API == ANS
    return anssock_epoll_create(512);
#elif API == FSTACK
    return ff_epoll_create(1);
#else
#error "Specify API"
#endif
}

int PLinkBind(int sockfd, const sockaddr *addr, socklen_t addrlen)
{
#if API == LINUX
    return bind(sockfd, addr, addrlen);
#elif API == ANS
    return anssock_bind(sockfd, addr, addrlen);
#elif API == FSTACK
    return ff_bind(sockfd, (linux_sockaddr *)addr, addrlen);
#else
#error "Specify API"
#endif
}

ssize_t PLinkRead(int fd, void *buf, size_t count)
{
#if API == LINUX
    return read(fd, buf, count);
#elif API == ANS
    return anssock_read(fd, buf, count);
#elif API == FSTACK
    return ff_read(fd, buf, count);
#else
#error "Specify API"
#endif
}

int PLinkEpollWait(int epfd, epoll_event *events, int maxevents, int timeout)
{
#if API == LINUX
    return epoll_wait(epfd, events, maxevents, timeout);
#elif API == ANS
    return anssock_epoll_wait(epfd, events, maxevents, timeout);
#elif API == FSTACK
    return ff_epoll_wait(epfd, events, maxevents, timeout);
#else
#error "Specify API"
#endif
}