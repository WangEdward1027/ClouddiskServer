#include "client.h"
#include "str_util.h"

#define BUFFER_SIZE 256


void receive_response(int sockfd, char *response) {
    int len = recv(sockfd, response, BUFFER_SIZE - 1, 0); // 0 表示默认标志位
    printf("response len: %d\n", len);
    if (len > 0) {
        response[len] = '\0';
    }
}

void encrypt_password(const char *password, const char *salt, char *encrypted_password) {
    char combined[BUFFER_SIZE];
    unsigned char hash[SHA256_DIGEST_LENGTH];

    // 将盐值和密码连接在一起
    snprintf(combined, sizeof(combined), "%s%s", password, salt);

    // 使用 SHA-256 哈希函数加密
    SHA256((unsigned char *)combined, strlen(combined), hash);

    // 把哈希值转换为十六进制字符串
    for (int i = 0; i < SHA256_DIGEST_LENGTH; i++) {
        snprintf(encrypted_password + (i * 2), 3, "%02x", hash[i]);
    }
    encrypted_password[SHA256_DIGEST_LENGTH * 2] = '\0'; // 确保字符串终止符
}

void clearInputBuffer() {
    int c;
    while ((c = getchar()) != '\n' && c != EOF);
}

void register_client(int sockfd) {
    char response[BUFFER_SIZE];
    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];
    char salt[256];
    char encrypted_password[BUFFER_SIZE];
    int length;
    CmdType cmdType;


    printf("请输出新的用户名：\n");
    scanf("%s",username);
    username[strcspn(username, "\n")] = 0;
    clearInputBuffer();
    //1.发送消息长度
    length=strlen(username);
    send(sockfd,&length,sizeof(length),0);

    //2.发送消息类型
    cmdType=CMD_TYPE_REGISTER_USERNAME;
    send(sockfd, &cmdType,sizeof(cmdType),0);
    
    //3.发送用户信息
    // 填充结构体成员
     User* user;
    // 为 User 结构体分配内存
    user = (User*)malloc(sizeof(User));
    if (user == NULL) {
        perror("Failed to allocate memory");
        exit(0);
    }

    // 将结构体初始化为 0
    memset(user, 0, sizeof(User));

    // 填充结构体成员
    user->id = 1;
    strcpy(user->userName, username);
    strcpy(user->salt, "salt");
    strcpy(user->cryptpasswd, "cryptpasswd");
    strcpy(user->pwd, "pwd");


    int ret = send(sockfd,user,sizeof(User),0);
    printf("data ret:%d\n", ret);
    printf("----------------------------------\n");  
    //4.发送消息内容
    //snprintf(request, sizeof(request), "CMD_TYPE_REGISTER_USRNAME:%s", username);
    //send(sockfd, request,strlen(request),0);
    
    // 接收服务器响应
    receive_response(sockfd, response);
    
    // 盐值检查
    if (strstr(response, "SALT")) {
        sscanf(response, "SALT:%s", salt);
        // 提示用户输入密码
        printf("请输入密码：\n");
        scanf("%s",password);
        password[strcspn(password, "\n")] = 0; // 去除换行符     
        clearInputBuffer();
        // 盐值加密
        encrypt_password(password, salt, encrypted_password);

        //1.发送消息长度
        length=strlen(encrypted_password);
        printf("2 length: %d\n", length);
        send(sockfd,&length,sizeof(length),0);

        //2.发送消息类型
        cmdType=CMD_TYPE_REGISTER_ENCRYTPTEDCODE;
        send(sockfd,&cmdType,sizeof(cmdType),0);

        //3.发送用户信息
        memset(user,0,sizeof(User));

        strcpy(user->userName,username); 
        strcpy(user->salt,salt);
        strcpy(user->cryptpasswd,encrypted_password);
        strcpy(user->pwd,username);
        send(sockfd,user,sizeof(User),0);

        // 发送加密后的密码
        //memset(request,0,sizeof(request));
        //snprintf(request, sizeof(request), "CMD_TYPE_ENCRYPTECODE:%s", encrypted_password);
        //int ret = send(sockfd, request,strlen(request),0);

        // 接收服务器最终响应
        receive_response(sockfd, response);
        if (strstr(response, "MSG_TYPE_REGISTEROK")) {
            printf("注册成功\n");
            free(user);
        } else {
            printf("注册失败，错误代码：%s\n", response);
            free(user);
        }
    } else {
        printf("用户已存在。\n");
        free(user);
    }
}

