#ifndef WEBSERVER_H
#define WEBSERVER_H

#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <stdio.h>
#include <unistd.h>
#include <errno.h>
#include <fcntl.h>
#include <stdlib.h>
#include <cassert>
#include <sys/epoll.h>
#include <string>
#include <cstring>

#include "../common/http/http_conn.h"
#include "../common/timer/timer.h"
#include "../common/threadpool.h"

const int MAX_FD = 65536;           //max file description
const int MAX_EVENT_NUMBER = 10000; //max event number
const int TIMESLOT = 5;             //min timeout unit


class WebServer
{
public:
    WebServer();
    ~WebServer();

    void init(int port, int actor_model);
    void WebServer::thread_pool();
    void event_listen();
    void event_loop();

private:
    bool WebServer::dealWithSignal(bool &timeout, bool &stop_server);
    void dealTimer(util_timer *timer, int sockfd);
    void WebServer::adjustTimer(util_timer *timer);
    void dealWithRead(int sockfd);
    void dealWithWrite(int sockfd);
    void addClient(int connfd, struct sockaddr_in client_addr);
    bool dealClientData();

public:
    //basic information
    int m_port;
    char *m_root;
    int m_actormodel;

    int m_pipe_fd[2];
    int m_epoll_fd;

    //线程池相关
    threadpool<http_conn> *m_pool;
    int m_thread_num;

    //epoll_event unit
    epoll_event events[MAX_EVENT_NUMBER];

    int m_listen_fd;

    http_conn *users;
    client_data *client;
    Utils utils;
};

#endif