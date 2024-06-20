#pragma once

#ifndef __CLIENT_H__
#define __CLIENT_H__

#define _CLIENT_SOURCE
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
#include <mysql/mysql.h>
#include <openssl/evp.h>
#include <openssl/sha.h>
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



#define USER_NAME "please input a valid user name:\n"
#define PASSWORD "please input the right password:\n"
typedef enum {
    CMD_TYPE_PWD=1,
    CMD_TYPE_LS,
    CMD_TYPE_CD,
    CMD_TYPE_MKDIR,
    CMD_TYPE_RMDIR,
    CMD_TYPE_PUTS,
    CMD_TYPE_GETS,
    CMD_TYPE_TOUCH,
    CMD_TYPE_REMOVE,
    CMD_TYPE_NOTCMD,  //不是命令

    //user login
    CMD_TYPE_LOGIN_USRNAME = 100,
    CMD_TYPE_REGISTER_USERNAME,    
    CMD_TYPE_LOGIN_ENCRYTPTEDCODE,
    CMD_TYPE_REGISTER_ENCRYTPTEDCODE,
    MSG_TYPE_LOGIN_SALT,
    MSG_TYPE_REGISTER_SALT,
    MSG_TYPE_LOGINOK,
    MSG_TYPE_LOGINERROR,
    MSG_TYPE_REGISTEROK,
    MSG_TYPE_REGISTERERROR,

}CmdType;


//1.用户注册表
typedef struct User{
    int id;
    char userName[64];
    char salt[64];
    char cryptpasswd[64];
    char pwd[64];
}User;


typedef struct 
{
    int len;//记录内容长度
    CmdType type;//消息类型
    User* user;  //对应用户信息
    char buff[1000];//记录内容本身
}train_t;

int tcpConnect(const char * ip, unsigned short port);
int recvn(int sockfd, void * buff, int len);
int sendn(int sockfd, const void * buff, int len);

//登录和注册，0成功，1失败
int login_client(int sockfd, User* user);
void register_client(int sockfd);

void receive_response(int sockfd, char *response);
void encrypt_password(const char *password, const char *salt, char *encrypted_password);

//命令解析
int parseCommand(const char * input, int len, train_t * pt, const User* user);


//执行上传文件操作
void putsCommand(int sockfd, train_t * pt);
//将本地文件上传至服务器
void putsFile(int sockfd, train_t * pt);
//从服务器下载文件
void getsFile(int scokfd);
//监听事件
int addEpollReadfd(int epfd, int fd);
int delEpollReadfd(int epfd, int fd);
//大文件传输
int transferFile(int sockfd);

//登录模块
void login_view(int sockfd, User* user);
#endif
