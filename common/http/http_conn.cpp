#include "http_conn.h"

int http_conn::h_epoll_fd = 0;
int http_conn::m_user_count = 0;