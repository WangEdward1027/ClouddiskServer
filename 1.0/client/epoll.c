#include "client.h"

int addEpollReadfd(int epfd, int fd){
    struct epoll_event event;
    event.events = EPOLLIN;
    event.data.fd = fd;
    int ret = epoll_ctl(epfd,EPOLL_CTL_ADD,fd,&event);
    ERROR_CHECK(ret,-1,"epollAdd");
    return 0;
}

int delEpollReadfd(int epfd, int fd){
    int ret = epoll_ctl(epfd,EPOLL_CTL_DEL,fd,NULL);
    ERROR_CHECK(ret,-1,"epollDel");
    return 0;
}
