#include "client.h"
#include "str_util.h"

#define BUFFER_SIZE 256

int login_client(int sockfd, User* user){
    char request[BUFFER_SIZE];
    char response[BUFFER_SIZE];
    char username[BUFFER_SIZE];
    char password[BUFFER_SIZE];
    char salt[256];
    char encrypted_password[BUFFER_SIZE];
    
    printf("请输入用户名：\n");
    fgets(username, sizeof(username), stdin);
    username[strcspn(username, "\n")] = 0;
    //登录第一次交互，只填充User结构体的用户名
    strncpy(user->userName, username, sizeof(user->userName)); //2填充用户名
    
    snprintf(request, sizeof(request), "CMD_TYPE_REGISTER_USRNAME:%s", username);
    send(sockfd, request,strlen(request),0);

    // 接收服务器响应
    receive_response(sockfd, response);

    // 盐值检查
    if (strstr(response, "SALT")) {
        sscanf(response, "SALT:%s", salt);
        printf("盐值：%s\n", salt);

        // 提示用户输入密码
        printf("请输入密码：\n");
        fgets(password, sizeof(password), stdin);
        password[strcspn(password, "\n")] = 0; // 去除换行符

        // 盐值加密
        encrypt_password(password, salt, encrypted_password);
        // 发送加密后的密码
        snprintf(request, sizeof(request), "CMD_TYPE_ENCRYPTECODE:%s", encrypted_password);
        strcpy(user->cryptpasswd, encrypted_password);  // 填充加密密码
        send(sockfd, request,strlen(request),0);

        // 接收服务器最终响应
        receive_response(sockfd, response);
        if(strstr(response, "MSG_TYPE_LOGINOK")) {
            printf("登录成功\n");
            return 0;
        }else {
            printf("登录失败，错误代码：%s\n", response);
            return 1;
        }
    }else{
        printf("该用户不存在。\n");
        return 1;
    }
}
