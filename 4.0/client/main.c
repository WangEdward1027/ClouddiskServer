#include "client.h"

#define EPOLL_ARR_SIZE 100

int main()
{
    int clientfd = tcpConnect("127.0.0.1", 8080);
    printf("欢迎使用“龟速网盘”\n");
    
    //登录模块,需要负责把Token填满
    User user;
    char Token[128];
    login_view(clientfd, &user, Token);
    printf("---------------请输入命令-----------------\n");
    
    //创建线程池结构体
    threadpool_t threadpool;
    memset(&threadpool, 0, sizeof(threadpool));
    //初始化线程池
    threadpoolInit(&threadpool, 3);
    //启动线程池
    threadpoolStart(&threadpool);

    char buf[1024] = {0};
    //使用epoll进行监听
    train_t train;

    //创建epoll实例
    int epfd = epoll_create1(0);
    ERROR_CHECK(epfd, -1, "epoll_create1");

    //对stdin和clientfd进行监听
    addEpollReadfd(epfd, STDIN_FILENO);
    addEpollReadfd(epfd, clientfd);

    struct epoll_event * pEventArr = (struct epoll_event*)
        calloc(EPOLL_ARR_SIZE, sizeof(struct epoll_event));

    while(1) {
        int nready = epoll_wait(epfd, pEventArr, EPOLL_ARR_SIZE, -1);
        if(nready == -1 && errno == EINTR) {
            continue;
        } else if(nready == -1) {
            ERROR_CHECK(nready, -1, "epoll_wait");
        } else {
            //大于0
            for(int i = 0; i < nready; ++i) {
                int fd = pEventArr[i].data.fd;
                if(fd == STDIN_FILENO) {
                    //读取标准输入中的数据
                    memset(buf, 0, sizeof(buf));
                    int ret = read(STDIN_FILENO, buf, sizeof(buf));
                    if(0 == ret || strcmp(buf, "exit")) {
                        //主线程通知所有的子线程退出
                        threadpoolStop(&threadpool);
                        //子进程退出前，回收资源
                        threadpoolDestroy(&threadpool);
                        close(clientfd);
                        close(epfd);
                        printf("byebye.\n");
                        exit(0);
                    }
                    memset(&train, 0, sizeof(train));
                    //解析命令行
                    buf[strlen(buf)-1] = '\0';
                    parseCommand(buf, strlen(buf), &train, &user);
                    //修改发送内容的类型
                    sendn(clientfd, &train, 4 + 4 + sizeof(User) + train.len);
                    //长命令交给子线程
                    if(train.type==CMD_TYPE_GETS || train.type==CMD_TYPE_PUTS){
                        //往任务队列里添加任务
                        task_t *ptask = (task_t*)calloc(1, sizeof(task_t));
                        ptask->type = train.type;
                        strcpy(ptask->Token, Token);
                        ptask->train = train;
                        taskEnque(&threadpool.que, ptask);
                    }
                }
                //接受短命令执行后服务器返回数据并打印 
                else if(fd == clientfd) {
                    //先接受返回内容长度
                    int length = -1;
                    recvn(clientfd, &length, sizeof(length));
                    //再接受返回内容
                    char buff1[4096];
                    memset(&buff1,0,sizeof(buff1));
                    recvn(clientfd, &buff1, length);
                    printf("%s\n",buff1);//接收短命令执行结果
                    printf("---------------命令执行完毕------------------\n");
                }
            }
       }
    }
    close(clientfd);
    return 0;
}
