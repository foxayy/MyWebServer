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
    util_timer *timer = new util_timer;
    timer->user_data = &client[connfd];
    timer->cb_func = cb_client_disconn;
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    client[connfd].timer = timer;
    utils.m_timer_lst.add_timer(timer);

    // show client info
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
    //utils.add_fd(m_epoll_fd, connfd, true, 0);

    return true;
}

bool WebServer::dealWithSignal(bool &timeout, bool &stop_server)
{
    int ret = 0;
    int sig;
    char signals[1024];
    ret = recv(m_pipe_fd[0], signals, sizeof(signals), 0);
    if (ret == -1)
    {
        return false;
    }
    else if (ret == 0)
    {
        return false;
    }
    else
    {
        for (int i = 0; i < ret; ++i)
        {
            switch (signals[i])
            {
            case SIGALRM:
            {
                timeout = true;
                break;
            }
            case SIGTERM:
            {
                stop_server = true;
                break;
            }
            }
        }
    }
    return true;
}

void WebServer::dealWithRead(int sockfd)
{
    util_timer *timer = client[sockfd].timer;
    if (timer) {
        adjustTimer(timer);
    }

    // when receive read event, put the event into request queue
    m_pool->append(&users[sockfd], 0);

    while (true) {
        // 'improv' flag indicates that the current request has already been processed
        if (1 == users[sockfd].improv) {
            // 'timer_flag' indicates that request need to be processed by timer(error\close.etc)
            if (1 == users[sockfd].timer_flag) {
                dealTimer(timer, sockfd);
                users[sockfd].timer_flag = 0;
            }
            users[sockfd].improv = 0;
            break;
        }
    }
}

void WebServer::dealWithWrite(int sockfd)
{
    util_timer *timer = client[sockfd].timer;
    if (timer) {
        adjustTimer(timer);
    }

    // when receive read event, put the event into request queue
    m_pool->append(&users[sockfd], 1);

    while (true) {
        // 'improv' flag indicates that the current request has already been processed
        if (1 == users[sockfd].improv) {
            // 'timer_flag' indicates that request need to be processed by timer(error\close.etc)
            if (1 == users[sockfd].timer_flag) {
                dealTimer(timer, sockfd);
                users[sockfd].timer_flag = 0;
            }
            users[sockfd].improv = 0;
            break;
        }
    }
}

void WebServer::dealTimer(util_timer *timer, int sockfd)
{
    timer->cb_func(&client[sockfd]);
    if(timer) {
        utils.m_timer_lst.del_timer(timer);
    }

    printf("close fd %d\n", client[sockfd].sockfd);
}

void WebServer::adjustTimer(util_timer *timer)
{
    time_t cur = time(NULL);
    timer->expire = cur + 3 * TIMESLOT;
    utils.m_timer_lst.adjust_timer(timer);

    printf("%s\n", "adjust timer once");
}

/********************************************/
/************ outter functions **************/
/********************************************/

void WebServer::init(int port, int actor_model)
{
    // set member variables
    m_port = port;
    m_actormodel = actor_model;
}

void WebServer::thread_pool()
{
    //thread pool
    m_pool = new threadpool<http_conn>(m_actormodel, m_thread_num);
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

    utils.init(TIMESLOT);

    utils.add_fd(m_epoll_fd, m_listen_fd, false, 0);
    http_conn::h_epoll_fd = m_epoll_fd;

    ret = socketpair(PF_UNIX, SOCK_STREAM, 0, m_pipe_fd);
    assert(ret != -1);
    utils.setNonBlocking(m_pipe_fd[1]);
    utils.add_fd(m_epoll_fd, m_pipe_fd[0], false, 0);

    utils.add_sig(SIGPIPE, SIG_IGN);
    utils.add_sig(SIGALRM, utils.sig_handler, false);
    utils.add_sig(SIGTERM, utils.sig_handler, false);

    alarm(TIMESLOT);

    //工具类,信号和描述符基础操作
    Utils::u_pipe_fd = m_pipe_fd;
    Utils::u_epoll_fd = m_epoll_fd;

}

void WebServer::event_loop()
{
    bool timeout = false;
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
                if(false == ret)
                    continue;

            } else if (events[i].events & (EPOLLRDHUP | EPOLLHUP | EPOLLERR)) {
                // handle error event
                util_timer *timer = client[sockfd].timer;
                dealTimer(timer, sockfd);
            
            } else if ((sockfd == m_pipe_fd[0]) && (events[i].events & EPOLLIN)) {
                bool flag = dealWithSignal(timeout, stop_server);
                if (false == flag)
                    printf("%s\n", "dealclientdata failure");

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