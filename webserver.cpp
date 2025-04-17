#include "webserver.h"

/********************************************/
/************ inner functions ***************/
/********************************************/

WebServer::WebServer()
{

}


WebServer::~WebServer()
{
    close(m_epoll_fd);
    close(m_listen_fd);
}

bool WebServer::dealClientData()
{
    struct sockaddr_in client_addr;
    socklen_t client_addr_len = sizeof(client_addr);

    int connfd = accept(m_listen_fd, (struct sockaddr *)&client_addr, &client_addr_len);
    if (connfd < 0)
    {
        printf("%s:errno is:%d", "accept error", errno);
        return false;
    }

    return true;
}


/********************************************/
/************ outter functions **************/
/********************************************/

void WebServer::init(int port)
{
    // set member variables
    m_port = port;

}

void WebServer::event_listen()
{
    // create socket
    m_listen_fd = socket(PF_INET, SOCK_STREAM, 0);
    assert(m_listen_fd >= 0);

    int ret = 0;
    struct sockaddr_in address;
    bzero(&address, sizeof(address));
    address.sin_family = AF_INET;
    address.sin_addr.s_addr = htonl(INADDR_ANY);
    address.sin_port = htons(m_port);

    int flag = 1;
    setsockopt(m_listen_fd, SOL_SOCKET, SO_REUSEADDR, &flag, sizeof(flag));
    ret = bind(m_listen_fd, (struct sockaddr *)&address, sizeof(address));
    assert(ret >= 0);
    ret = listen(m_listen_fd, 5);
    assert(ret >= 0);

    // epoll create kernel event table
    m_epoll_fd = epoll_create(1);
    assert(m_epoll_fd != -1);
}

void WebServer::event_loop()
{
    bool stop_server = false;
    while(!stop_server) {
        int number = epoll_wait(m_epoll_fd, events, MAX_EVENT_NUMBER, -1);

        if(number < 0 && errno != EINTR) {
            printf("epoll failure\n");
            break;
        }

        for(int i = 0; i < number; i++) {
            int sockfd = events[i].data.fd;
            if (sockfd == m_listen_fd) {
                // handle new connection
                bool ret = 
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                // handle error event

            } else if (events[i].events & EPOLLIN) {
                // handle read event

            } else if (events[i].events & EPOLLOUT) {
                // handle write event

            }
        }
    }
}