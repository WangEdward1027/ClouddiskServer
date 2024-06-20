#include "client.h"
#include "str_util.h"

int tcpConnect(const char * ip, unsigned short port)
{
    //1. 创建TCP的客户端套接字
    int clientfd = socket(AF_INET, SOCK_STREAM, 0);
    if(clientfd < 0) {
        perror("socket");
        return -1;
    }

    //2. 指定服务器的网络地址
    struct sockaddr_in serveraddr;
    //初始化操作,防止内部有脏数据
    memset(&serveraddr, 0, sizeof(serveraddr));
    serveraddr.sin_family = AF_INET;//指定IPv4
    serveraddr.sin_port = htons(port);//指定端口号
    //指定IP地址
    serveraddr.sin_addr.s_addr = inet_addr(ip);

    //3. 发起建立连接的请求
    int ret = connect(clientfd, (const struct sockaddr*)&serveraddr, sizeof(serveraddr));
    if(ret < 0) {
        perror("connect");
        close(clientfd);
        return -1;
    }
    return clientfd;
}

//其作用：确定接收len字节的数据
int recvn(int sockfd, void * buff, int len)
{
    int left = len;//还剩下多少个字节需要接收
    char * pbuf = (char*)buff;
    int ret = -1;
    while(left > 0) {
        ret = recv(sockfd, pbuf, left, 0);
        if(ret == 0) {
            break;
        } else if(ret < 0) {
            perror("recv");
            return -1;
        }

        left -= ret;
        pbuf += ret;
    }
    //当退出while循环时，left的值等于0
    return len - left;
}

//作用: 确定发送len字节的数据
int sendn(int sockfd, const void * buff, int len)
{
    int left = len;
    const char * pbuf = (const char*)buff;
    int ret = -1;
    while(left > 0) {
        ret = send(sockfd, pbuf, left, 0);
        if(ret < 0) {
            perror("send");
            return -1;
        }

        left -= ret;
        pbuf += ret;
    }
    return len - left;
}

//解析命令
int parseCommand(const char* buff, int len, train_t* pt, User* user){
    //填写train结构体
    pt->len = strlen(buff);
    pt->user = user;

    //把buff里的第一个命令分词，然后判断CmdTpe的类型,填入train_t中
    char* tempbuff =(char *)calloc(len + 1, sizeof(char));
    strcpy(tempbuff, buff);     //buff是const
    char * token = strtok(tempbuff, " "); //按照空格进行分词
    
    if(token == NULL){
        free(tempbuff);
        return -1;  //解析失败
    }

    if(token == NULL) return -1;  //解析失败
    if(strcmp(token, "pwd") == 0){
        pt->type = CMD_TYPE_PWD;
    }else if(strcmp(token, "ls") == 0){
        pt->type = CMD_TYPE_LS;
    }else if(strcmp(token, "cd") == 0){
        pt->type = CMD_TYPE_CD;
    }else if(strcmp(token, "mkdir") == 0){
        pt->type = CMD_TYPE_MKDIR;
    }else if(strcmp(token, "rmdir") == 0){
        pt->type = CMD_TYPE_RMDIR;
    }else if(strcmp(token, "puts") == 0){
        pt->type = CMD_TYPE_PUTS;
    }else if(strcmp(token, "gets") == 0){
        pt->type = CMD_TYPE_GETS;
    }else if(strcmp(token,"rm")==0){
        pt->type=CMD_TYPE_REMOVE;
    }
    else if(strcmp(token,"touch")==0){
        pt->type=CMD_TYPE_TOUCH;
    }
    else{
        pt->type = CMD_TYPE_NOTCMD;
    }
    
    //把剩余的字符串放进train_t 的 buff里
    token = strtok(NULL, " ");
    if(token != NULL){
        strcpy(pt->buff, token);
        while((token = strtok(NULL, " ")) != NULL) {
            strcat(pt->buff, " ");
            strcat(pt->buff, token);
        }
    } else {
        pt->buff[0] = '\0'; // 如果没有剩余字符串，则将buff清空
    }

    free(tempbuff);
    return 0;
}

void login_view(int sockfd, User* user){
    int user_choice;

    while(1){
        printf("————————登录界面——————————\n");
        printf("请输入选项序号：\n");
        printf("1.注册\n");
        printf("2.登录\n");
        printf("3.退出\n");
        printf("请输入您的选择：");

        if(scanf("%d", &user_choice) != 1) {
            // 清除输入缓冲区
            while(getchar() != '\n');
            printf("无效输入，请输入数字。\n");
            continue;
        }

        switch(user_choice){
        case 1:
            register_client(sockfd);
            break;
        case 2:
            if (login_client(sockfd, user)) {
                printf("登录成功\n");
                return; // 退出函数，结束循环
            } else {
                printf("登录失败，请重试。\n");
            }
            break;
        case 3:
            printf("正在退出...\n");
            exit(0); // 退出程序
        default:
            printf("输入错误，重新输入！\n");
            break;
        }
    }
}

//按文件内容生成对应的MD5值
void generateMD5(const char *filename, char *md5String) {
    EVP_MD_CTX *mdctx;
    const EVP_MD *md;
    unsigned char md_value[EVP_MAX_MD_SIZE];
    unsigned int md_len, i;
    FILE *file;
    int bytes;
    unsigned char data[1024];

    // 初始化上下文
    mdctx = EVP_MD_CTX_new();
    if (mdctx == NULL) {
        printf("EVP_MD_CTX_new failed\n");
        return;
    }

    // 使用MD5算法
    md = EVP_md5();
    if (EVP_DigestInit_ex(mdctx, md, NULL) != 1) {
        printf("EVP_DigestInit_ex failed\n");
        EVP_MD_CTX_free(mdctx);
        return;
    }

    // 打开文件
    file = fopen(filename, "rb");
    if (file == NULL) {
        printf("%s can't be opened.\n", filename);
        EVP_MD_CTX_free(mdctx);
        return;
    }

    // 读取文件并更新MD5上下文
    while ((bytes = fread(data, 1, 1024, file)) != 0) {
        if (EVP_DigestUpdate(mdctx, data, bytes) != 1) {
            printf("EVP_DigestUpdate failed\n");
            fclose(file);
            EVP_MD_CTX_free(mdctx);
            return;
        }
    }

    // 完成哈希计算
    if (EVP_DigestFinal_ex(mdctx, md_value, &md_len) != 1) {
        printf("EVP_DigestFinal_ex failed\n");
        fclose(file);
        EVP_MD_CTX_free(mdctx);
        return;
    }

    // 释放上下文
    EVP_MD_CTX_free(mdctx);
    fclose(file);

    // 将MD5值转换为十六进制字符串
    for (i = 0; i < md_len; i++) {
        sprintf(&md5String[i * 2], "%02x", md_value[i]);
    }
    md5String[md_len * 2] = '\0';
}
