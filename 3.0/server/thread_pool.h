#ifndef __THREAD_POOL_H__
#define __THREAD_POOL_H__

#define _SERVER_SOURCE
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


//1.用户注册表
typedef struct User{
    int id;
    char userName[64];
    char salt[64];
    char cryptpasswd[64];
    char pwd[64];
}User;

//2.服务器文件表
typedef struct FileInfo{
    char md5[64];
    char fileName[64];
}FileInfo;

//3.虚拟文件表
typedef struct FileEntry{
    int id;
    int parentId;
    char fileName[64];
    int ownerId;
    char md5[64];
    int fileSize;
    int fileType;
}FileEntry;

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


typedef struct 
{
    int len;//记录内容长度
    CmdType type;
    char buff[1000];//记录内容本身
}train_t;

typedef struct task_s{
    int peerfd;//与client进行通信的套接字
    int epfd;//epoll的实例
    CmdType type;
    char data[1000];
    User* user;
    struct task_s * pNext;
}task_t;

typedef struct task_queue_s
{
    task_t * pFront;
    task_t * pRear;
    int queSize;//记录当前任务的数量
    pthread_mutex_t mutex; 
    pthread_cond_t cond;
    int flag;//0 表示要退出，1 表示不退出

}task_queue_t;

typedef struct threadpool_s {
    pthread_t * pthreads;
    int pthreadNum;
    task_queue_t que;//...任务队列
}threadpool_t;

int queueInit(task_queue_t * que);
int queueDestroy(task_queue_t * que);
int queueIsEmpty(task_queue_t * que);
int taskSize(task_queue_t * que);
int taskEnque(task_queue_t * que, task_t * task);
task_t * taskDeque(task_queue_t * que);
int broadcastALL(task_queue_t* que);

int threadpoolInit(threadpool_t *, int num);
int threadpoolDestroy(threadpool_t *);
int threadpoolStart(threadpool_t *);
int threadpoolStop(threadpool_t *);


int tcpInit(const char * ip, const char * port);
int addEpollReadfd(int epfd, int fd);
int delEpollReadfd(int epfd, int fd);
int transferFile(int sockfd);
int sendn(int sockfd, const void * buff, int len);
int recvn(int sockfd, void * buff, int len);

//处理客户端发过来的消息
void handleMessage(int sockfd, int epfd, task_queue_t * que);
//执行任务的总的函数
void doTask(task_t * task);
//每一个具体命令的执行
void cdCommand(task_t * task);
void lsCommand(task_t * task);
void pwdCommand(task_t * task);
void mkdirCommand(task_t * task);
void rmdirCommand(task_t * task);
void notCommand(task_t * task);
void putsCommand(task_t * task);
void getsCommand(task_t * task);
void touchCommand(task_t * task);
void removeCommand(task_t * task);

//用户登录的操作
void userLoginCheck1(task_t * task);
void userLoginCheck2(task_t * task);

//数据库，返回0成功，返回-1失败，实体返回NULL为失败
int addUser(User* user);
//int deleteUser(int userId);
User* selectUser(int userId);
//int updateUser(User* user);
User* selectUserByUserName(const char* userName);
int addFileInfo(FileInfo* fileInfo);
FileInfo* selectFileInfo(char* md5, int md5Len);

int addFileEntry(FileEntry* fileEntry);
FileEntry* selectFileEntry(int id);
int updateFileEntry(FileEntry* fileEntry);
int deleteFileEntry(int id);
FileEntry* selectFileEntryByFileName(const char* fileName);
FileEntry* selectFileEntryByOwnerId(int ownerId, int* entryCount);
FileEntry* selectFileEntryByparentId(int parentId);
FileEntry* selectFileEntryByFileNameAndOwnerId(const char* fileName, int ownerId, int* entryCount);

// 获取pwd的最后一个目录项和倒数第二个目录项
char* getCurrentDirectory(const char* pwd);
char* getParentDirectory(const char* pwd);

//用户注册
void register_server(task_t * task);
void userRegister1(task_t* task);
void userRegister2(task_t* task);

//对指定文件生成相应的MD5值
void generateMD5(const char *filename, char *md5String);

MYSQL* create_db_connection();
#endif
