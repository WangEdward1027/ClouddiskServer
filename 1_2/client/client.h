#ifndef __WD_CLIENT_H
#define __WD_CLIENT_H

#define _GNU_SOURCE
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>
#include <error.h>
#include <sys/mman.h>
#include <sys/wait.h>
#include <time.h>
#include <unistd.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <signal.h>
#include <dirent.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <netdb.h>
#include <sys/epoll.h>
#include <assert.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <pthread.h>
#include <sys/uio.h>
#include <sys/sendfile.h>

#define SIZE(a) (sizeof(a)/sizeof(a[0]))

typedef void (*sighandler_t)(int);

#define ARGS_CHECK(argc, num)   {\
    if(argc != num){\
        fprintf(stderr, "ARGS ERROR!\n");\
        return -1;\
    }}

#define ERROR_CHECK(ret, num, msg) {\
    if(ret == num) {\
        perror(msg);\
        return -1;\
    }}

#define THREAD_ERROR_CHECK(ret, func) {\
    if(ret != 0) {\
        fprintf(stderr, "%s:%s\n", func, strerror(ret));\
    }}

typedef enum {
    CMD_TYPE_PWD=1,
    CMD_TYPE_LS,
    CMD_TYPE_CD,
    CMD_TYPE_MKDIR,
    CMD_TYPE_RMDIR,
    CMD_TYPE_PUTS,
    CMD_TYPE_GETS,
    CMD_TYPE_TREE,
    CMD_TYPE_USRNAME,
    CMD_TYPE_USRENCODE,
    CMD_TYPE_USRCHECK,
    CMD_TYPE_NOTCMD  //不是命令

}CmdType;

typedef struct 
{
    int len;//记录内容长度
    CmdType type;
    char buff[1000];//记录内容本身
}train_t;

typedef struct task_s{
    int peerfd;
    CmdType type;
    char data[1000];
    struct task_s * pNext;
}task_t;


int addEpollReadfd(int epfd, int fd);
int delEpollReadfd(int epfd, int fd);
int transferFile(int sockfd);
//客户端向服务器发送数据
int sendn(int sockfd, const void * buff, int len);

//接收服务器返回的数据
int recvn(int sockfd, void * buff, int len);

//将本地文件上传至服务器
void puts(task_t * task);  // 有问题,客户端没有task
//从服务器下载文件
void gets(int scokfd);
//客户端登录校验
int usrCheck(int sockfd);
//TCP连接
int tcpConnect(const char *ip,unsigned short port);

//建立tcp连接
int tcpConnect(const char *ip,unsigned short port);

//分词
int parseCommand(const char*buff,int len,train_t*pt);

#endif
