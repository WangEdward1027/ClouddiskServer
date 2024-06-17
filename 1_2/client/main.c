#include "client.h"

void sendtrain(int peerfd, train_t* train, int len) {

    // 1. 先发送len
    int contLen = train->len;
    send(peerfd, &contLen, sizeof(contLen), 0);

    // 2. 发送cmdtype
    CmdType type = train->type;
    send(peerfd, &type, sizeof(type), 0);

    // 3. 发送内容
    sendn(peerfd, train->buff, train->len);

}

int main()
{
    //tcp连接
    int clientfd=tcpConnect("127.0.0.1",8080);//int tcp(const char *ip,unsigned short port)
    
    //登录
    usrCheck(clientfd);

    char buff[128]={0};

    //select监听
    fd_set rdset;
    train_t train;

    while(1){
        //初始化fd_set
        FD_ZERO(&rdset);
        FD_SET(STDIN_FILENO,&rdset);//将标准输入加入监听集合
        FD_SET(clientfd,&rdset);//将客户端套接字加入监听集合

        //调用select进行多路复用监听
        int nready=select(clientfd+1,&rdset,NULL,NULL,NULL);
        printf("nready:%d\n",nready);

        //检查用户是否输入
        if(FD_ISSET(STDIN_FILENO,&rdset)){
            //读取数据
            memset(buff,0,sizeof(buff));
            int ret=read(STDIN_FILENO,buff,sizeof(buff));
            if(ret==0){
                printf("再见。\n");
                break;
            }//用户ctrl c退出

            //正常输入
            memset(&train,0,sizeof(train));
            //解析命令行
            buff[strlen(buff)-1]='\0';//忽略换行符
            //分词解析命令
            parseCommand(buff,strlen(buff)-1,&train);//int parseCommand(const char*buff,int len,train_t*pt)
            sendtrain(clientfd,&train,4+4+train.len);
        }

        //接收服务器数据
        else if(FD_ISSET(clientfd,&rdset)){
            int recvCmdType;
            recv(clientfd,&recvCmdType,sizeof(recvCmdType),0);//接收命令类型

            printf("命令是：%d号命令\n",recvCmdType);
            //接收文件是9号命令：接收文件
            if(recvCmdType==9){
                /* recvFile(clientfd);//int recvFile(int sockfd)接收文件 */
                getsCommand(clientfd);
            }
            
            //执行普通命令
            else{
                char buff[1024];
                recv(clientfd,&buff,sizeof(buff),0);
                printf("%s\n",buff);//接收命令执行结果
            }
        }

    }
    close(clientfd);
    return 0;
}

