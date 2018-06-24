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

struct epoll_event ev;
struct epoll_event events[MAX_EVENTS];

int epfd;
int sockfd;

int main(int argc, char *argv[])
{
    ff_init(argc, argv);
    //find --
    int skip = 0;
    for (int i = 0; i < argc; i++)
    {
        if(strcmp("--", argv[i]) == 0)
        {
            skip == i + 1;
            break;
        }
    }

    ArgumentParser ap;
    ap.addArgument("--serverIp",1, false);
    ap.addArgument("--duration",1, true);
    ap.addArgument("--interval",1, true);
    argc -= skip;
    argv += skip;

    ap.parse(argc, (const char**)argv);


    int duration = 10;
    int interval = 1;
    std::string sip;
    
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
    ff_connect(sockfd, (sockaddr_in *)&my_addr, sizeof(sockaddr_in));
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
