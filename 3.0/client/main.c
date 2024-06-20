#include "client.h"

int main()
{
    int clientfd = tcpConnect("127.0.0.1", 8080);
    printf("欢迎使用“龟速网盘”\n");
    
    //登录模块
    User user;
    login_view(clientfd, &user);

    char buf[1024] = {0};
    //4. 使用select进行监听
    fd_set rdset;
    train_t train;
    train.user = &user;

    while(1) {
        FD_ZERO(&rdset);
        FD_SET(STDIN_FILENO, &rdset);
        FD_SET(clientfd, &rdset);
        
        int nready = select(clientfd + 1, &rdset, NULL, NULL, NULL);
        printf("nready:%d\n", nready);
        if(FD_ISSET(STDIN_FILENO, &rdset)) {
            //读取标准输入中的数据
            memset(buf, 0, sizeof(buf));
            int ret = read(STDIN_FILENO, buf, sizeof(buf));
            if(0 == ret) {
                printf("byebye.\n");
                break;
            }
            memset(&train, 0, sizeof(train));
            //解析命令行
            buf[strlen(buf)-1] = '\0';
            parseCommand(buf, strlen(buf), &train, &user);
            //TODO 修改发送内容的类型
            sendn(clientfd, &train, 4 + 4 + sizeof(User) + train.len);
            if(train.type == CMD_TYPE_PUTS) {
                putsCommand(clientfd, &train);
            }
        }//接受服务器返回数据并打印 
        else if(FD_ISSET(clientfd, &rdset)) {
            int recvCmdType=train.type;

            char buff1[4096];
            //接收文件是接收文件命令
            if(recvCmdType==CMD_TYPE_GETS){
                getsCommand(clientfd, &train);
            }else if(recvCmdType==CMD_TYPE_PUTS){
                printf("开始发送文件\n");
                putsCommand(clientfd, &train);
            }

            //执行普通命令
            else{
                memset(&buff1,0,sizeof(buff1));
                recv(clientfd,&buff1,sizeof(buff1),0);
                printf("%s\n",buff1);//接收命令执行结果
            }

            printf("---------------命令执行完毕------------------\n");
        }
    }
    close(clientfd);
    return 0;
}

