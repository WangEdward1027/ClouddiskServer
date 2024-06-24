#include "client.h"
#include "str_util.h"
#define NUM_STARS 1000  // 调整此值以生成更多或更少的字符
#define NUM_COLORS 8    // 使用的颜色数量
#define QR_SIZE 33
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
    //填写train内的User结构体
    pt->user.id = user->id;
    printf("username: %s\n", user->userName);
    strcpy(pt->user.userName,user->userName);
    strcpy(pt->user.salt,user->salt);
    strcpy(pt->user.cryptpasswd,user->cryptpasswd);
    strcpy(pt->user.pwd,user->pwd);

    //把buff里的第一个命令分词，然后判断CmdTpe的类型,填入train_t中
    char* tokens[64]; //token字符数组
    int max_tockens = 10; //最大分词数
    int pcount; //分词后参数个数
    char delimiter[8] = " "; //分隔符

    //分词
    splitString(buff, delimiter, tokens, max_tockens, &pcount);

    //添加对应命令类型
    if(strcmp(tokens[0], "pwd") == 0){
        pt->type = CMD_TYPE_PWD;
    }else if(strcmp(tokens[0], "ls") == 0){
        pt->type = CMD_TYPE_LS;
    }else if(strcmp(tokens[0], "cd") == 0){
        pt->type = CMD_TYPE_CD;
    }else if(strcmp(tokens[0], "mkdir") == 0){
        pt->type = CMD_TYPE_MKDIR;
    }else if(strcmp(tokens[0], "rmdir") == 0){
        pt->type = CMD_TYPE_RMDIR;
    }else if(strcmp(tokens[0], "puts") == 0){
        pt->type = CMD_TYPE_PUTS;
    }else if(strcmp(tokens[0], "gets") == 0){
        pt->type = CMD_TYPE_GETS;
    }else if(strcmp(tokens[0],"rm")==0){
        pt->type=CMD_TYPE_REMOVE;
    }
    else if(strcmp(tokens[0],"touch")==0){
        pt->type=CMD_TYPE_TOUCH;
    }
    else{
        pt->type = CMD_TYPE_NOTCMD;
    }
   
    //存入参数和辅助数据长度
    for(int i = 1; i<pcount; i++)
    {
        strcat(tokens[i], " ");
        strcat(pt->buff, tokens[i]);
    }

    //存入发送的参数的数据的长度
    pt->len = strlen(pt->buff);

    //释放tokens空间
    freeStrs(tokens, pcount);
    return 0;
}
void printfstar();
 int qr_code[QR_SIZE][QR_SIZE] = {
 {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 1, 1, 1}, 
{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 0, 1, 0, 0}, 
{1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 1 }, 
{1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1}, 
{ 1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 0, 1 },
 {1, 0, 0, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0 },
 {1, 1, 1, 1, 1, 1, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0}, 
{0, 0, 0, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0, 0}, 
{1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0}, 
{ 1, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 1, 0, 1, 1, 0, 1}, 
{1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0}, 
{ 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1, 0 }, 
{ 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 0, 1, 0}, 
{ 0, 0, 1, 0, 1, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 1, 1}, 
{1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 0} ,
 {1, 1, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0},
 {0, 0, 1, 0, 1, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 0, 0, 1 }, 
{ 1, 1, 1, 1, 1, 1, 0, 0, 1, 1, 0, 0, 1, 1, 1, 0, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 1, 1, 1}, 
{1, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 0, 0, 1, 1, 1, 0, 1, 0, 0, 1, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 1, 0 }, 
{1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 0, 0, 1, 1, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1}, 
{ 0, 0, 0, 1, 1, 0, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 1, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 1, 1, 1, 0, 1, 1},
 { 1, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 0, 1, 0, 0, 1, 0, 0, 0, 0, 1, 0, 1, 1, 0, 0, 1, 1, 0, 1}, 
{1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 0 }, 
{ 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 0, 1, 0, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 1, 0, 1, 1, 0, 0}, 
{1, 0, 1, 0, 1, 0, 1, 1, 0, 0, 1, 0, 1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 1, 0, 0, 1, 1}, 
{0, 0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 0, 1, 0, 1},
 {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 1, 0, 1, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 1, 0},
 {1, 0, 0, 0, 0, 0, 1, 0, 1, 1, 0, 1, 0, 0, 0, 1, 0, 0, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 1, 1, 0, 0}, 
{1, 0, 1, 1, 1, 0, 1, 0, 1, 1, 0, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 1, 1, 1, 0, 1, 1}, 
{ 1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 0, 1, 0, 0}, 
{1, 0, 1, 1, 1, 0, 1, 0, 1, 0, 0, 1, 1, 0, 1, 0, 0, 1, 1, 1, 0, 1, 1, 0, 0, 0, 1, 1, 0, 1, 1, 0, 0}, 
{ 1, 0, 0, 0, 0, 0, 1, 0, 0, 0, 0, 0, 0, 1, 1, 0, 1, 0, 1, 1, 1, 1, 1, 1, 0, 0, 0, 0, 1, 1, 1, 0, 0},
 {1, 1, 1, 1, 1, 1, 1, 0, 0, 0, 1, 0, 1, 1, 1, 1, 0, 1, 1, 1, 1, 0, 0, 1, 0, 0, 1, 1, 0, 1, 0, 1, 0},

 };
void login_view(int sockfd, User* user){
    int user_choice;

    while(1){
        printf("\n\033[1;96m-----------登录界面-----------\033[0m \n");
        printf("\033[1;97m|请输入选项序号：            |\033[0m\n");
        printf("\033[1;97m|           1.注 册          |\033[0m\n");
        printf("\033[1;97m|           2.登 录          |\033[0m\n");
        printf("\033[1;97m|           3.退 出          |\033[0m\n");
        printf("\033[1;97m|                            |\033[0m\n");
        printf("\033[1;97m|\033[0m\033[1;5;97;41m      千万不要输入4！！！！ \033[0m\033[1;97m|\033\n");
        printf("\033[1;97m|\033\033[1;5;90;43m      5.查看代码仓库二维码  \033[0m\033[1;97m|\033\n");
        printf("\033[1;96m------------------------------\033[0m \n");
        printf("\033[1;97m请输入您的选择：\033[0m");
        if(scanf("%d", &user_choice) != 1) {
            // 清除输入缓冲区
            while(getchar() != '\n');
            printf("--------------------------\n");
            printf("\033[1;5;107;41m无效输入，请输入数字。\033[0m\n");
            continue;
        }

        switch(user_choice){
        case 1:
            register_client(sockfd);
            break;
        case 2:
            if (login_client(sockfd, user)) {
                printf("登录成功。\n");
                return; // 退出函数，结束循环
            } else {
                printf("登录失败，请重试。\n");
            }
            break;
        case 3:
            printf("用户已登出,再见。\n");
            exit(0); // 退出程序
        case 4:
            printfstar();
            break;
        case 5:
            printQRCode(QR_SIZE, qr_code);
            break;
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

void removeTrailingSpace(char* str) {
    int length = strlen(str);
    if (length > 0 && str[length - 1] == ' ') {
        str[length - 1] = '\0';
    }
}


const char* colors[NUM_COLORS] = {
    "30", "31", "32", "33", "34", "35", "36", "37"  // 黑色、红色、绿色、黄色、蓝色、紫色、青色、白色
};

const char* bg_colors[NUM_COLORS] = {
    "40", "41", "42", "43", "44", "45", "46", "47"  // 黑色、红色、绿色、黄色、蓝色、紫色、青色、白色
};

// 获取一个随机颜色和背景色，确保它们之间有较大反差
void get_random_color(char* color, char* bg_color) {
    int fg_index = rand() % NUM_COLORS;
    int bg_index = rand() % NUM_COLORS;

    // 确保前景色和背景色不同
    while (fg_index == bg_index) {
        bg_index = rand() % NUM_COLORS;
    }

    sprintf(color, "%s", colors[fg_index]);
    sprintf(bg_color, "%s", bg_colors[bg_index]);
}
void printfstar(){
    char color[3], bg_color[3];
    srand(time(NULL));  // 设置随机种子

    for (int i = 0; i < NUM_STARS; i++) {
        get_random_color(color, bg_color);
        printf("\033[5;%s;%sm叫你别点了\033[0m", color, bg_color);  // 使用闪烁 (5)

        // 每50个字符换行一次
        if ((i + 1) % 50 == 0) {
            printf("\n");
        }
}}

void printQRCode(int qr_size, int qr_code[qr_size][qr_size]) {
    for (int i = 0; i < qr_size; ++i) {
        for (int j = 0; j < qr_size; ++j) {
            if (qr_code[i][j] == 1) {
                printf("██");  // 使用两个字符来表示黑色模块
            } else {
                printf("  ");  // 使用两个空格来表示白色模块
            }
        }
        printf("\n");  // 换行
    }
}
