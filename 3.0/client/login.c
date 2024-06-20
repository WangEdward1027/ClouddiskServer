#include "client.h"
#include "str_util.h"
#include <openssl/sha.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>

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
        send(sockfd, request,strlen(request),0);

        // 接收服务器最终响应
        receive_response(sockfd, response);
        if (strstr(response, "MSG_TYPE_REGISTEROK")) {
            printf("登录成功\n");
            return 0;
        } else {
            printf("登录失败，错误代码：%s\n", response);
            return 1;
        }
    } else {
        printf("用户已存在。\n");
    }
}
