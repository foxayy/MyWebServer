#ifndef TIMER_H
#define TIMER_H

#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/epoll.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <sys/stat.h>
#include <string.h>
#include <pthread.h>
#include <stdio.h>
#include <stdlib.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include <sys/wait.h>
#include <sys/uio.h>

#include <time.h>


struct client_data
{
    sockaddr_in address;
    int sockfd;
};

class Utils
{
public:
    Utils() {}
    ~Utils() {}

    void init(int timeslot);

    //对文件描述符设置非阻塞
    int setNonBlocking(int fd);

    //将内核事件表注册读事件，ET模式，选择开启EPOLLONESHOT
    void add_fd(int epollfd, int fd, bool one_shot, int TRIGMode);
    void show_error(int connfd, const char *info);

public:
    static int u_epoll_fd;
    int m_TIMESLOT;
};

#endif