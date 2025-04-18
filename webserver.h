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

#include "./http/http_conn.h"


const int MAX_FD = 65536;           //max file description
const int MAX_EVENT_NUMBER = 10000; //max event number
const int TIMESLOT = 5;             //min timeout unit


class WebServer
{
public:
    WebServer();
    ~WebServer();

    void init(int port);

    void event_listen();
    void event_loop();

private:
    void dealWithRead(int sockfd);
    void dealWithWrite(int sockfd);
    void addClient(int connfd, struct sockaddr_in client_addr);
    bool dealClientData();

public:
    //basic information
    int m_port;
    char *m_root;

    //int m_pipefd[2];
    int m_epoll_fd;

    //epoll_event unit
    epoll_event events[MAX_EVENT_NUMBER];

    int m_listen_fd;

    client_data *client;
    Utils utils;
};

#endif