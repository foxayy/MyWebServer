#include "webserver.h"

/********************************************/
/************ inner functions ***************/
/********************************************/

WebServer::WebServer()
{
    users = new http_conn[MAX_FD];
    client = new client_data[MAX_FD];
}


WebServer::~WebServer()
{
    close(m_epoll_fd);
    close(m_listen_fd);

    delete[] users;
    delete[] client;
}

void WebServer::addClient(int connfd, struct sockaddr_in client_addr)
{
    users[connfd].init(connfd, client_addr);
    client[connfd].sockfd = connfd;
    client[connfd].address = client_addr;

    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client[connfd].address.sin_addr), 
              client_ip, INET_ADDRSTRLEN);

    printf("client %s:%d connected\n", client_ip,
          ntohs(client[connfd].address.sin_port));
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
    if (http_conn::m_user_count >= MAX_FD)
    {
        utils.show_error(connfd, "Internal server busy");
        printf("%s", "Internal server busy");
        return false;
    }

    addClient(connfd, client_addr);
    utils.add_fd(m_epoll_fd, connfd, true, 0);

    return true;
}

void WebServer::dealWithRead(int sockfd)
{
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client[sockfd].address.sin_addr), 
              client_ip, INET_ADDRSTRLEN);

    char buffer[1024];
    ssize_t bytes_read = read(sockfd, buffer, sizeof(buffer)-1);

    if (bytes_read <= 0) {
        if (bytes_read == 0) {
            printf("Client %s:%d disconnected\n", 
                  client_ip, ntohs(client[sockfd].address.sin_port));
        } else {
            perror("read error");
        }
        close(sockfd);
        return;
    }

    buffer[bytes_read] = '\0';
    printf("Received from %s:%d:\n%s\n", 
          client_ip, ntohs(client[sockfd].address.sin_port), 
          buffer);

    epoll_event ev;
    ev.data.fd = sockfd;
    ev.events = EPOLLOUT | EPOLLET;

    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, sockfd, &ev) == -1) {
        perror("epoll_ctl modify failed");
        close(sockfd);
    }
}

void WebServer::dealWithWrite(int sockfd)
{
    char client_ip[INET_ADDRSTRLEN];
    inet_ntop(AF_INET, &(client[sockfd].address.sin_addr), 
              client_ip, INET_ADDRSTRLEN);

    printf("Response sent to %s:%d\n", 
          client_ip, ntohs(client[sockfd].address.sin_port));

    const char* msg = "Hello World";
    ssize_t bytes_sent = write(sockfd, msg, strlen(msg));

    epoll_event ev;
    ev.data.fd = sockfd;
    ev.events = EPOLLIN | EPOLLET;

    if (epoll_ctl(m_epoll_fd, EPOLL_CTL_MOD, sockfd, &ev) == -1) {
        perror("epoll_ctl modify failed");
        close(sockfd);
    }
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

    utils.add_fd(m_epoll_fd, m_listen_fd, false, 0);
    http_conn::h_epoll_fd = m_epoll_fd;

    Utils::u_epoll_fd = m_epoll_fd;
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
                bool ret = dealClientData();
                if(false == ret) {
                    continue;
                }
            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                // handle error event

            } else if (events[i].events & EPOLLIN) {
                // handle read event
                dealWithRead(sockfd);
            } else if (events[i].events & EPOLLOUT) {
                // handle write event
                dealWithWrite(sockfd);
            }
        }
    }
}