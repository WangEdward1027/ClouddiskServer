#include "client.h"
#include "str_util.h"
#include <stdlib.h>

#define BUFFER_SIZE 256
void recvUser(int socket, User *user) {
    if (recvn(socket, user, sizeof(User)) == -1) {
        perror("recv");
        exit(EXIT_FAILURE);
    }
}

int login_client(int sockfd, User* user){
    char request[BUFFER_SIZE] = {0};
    char response[BUFFER_SIZE] = {0};
    char username[BUFFER_SIZE] = {0};
    char password[BUFFER_SIZE] = {0};
    char salt[256] = {0};
    char encrypted_password[BUFFER_SIZE] = {0};
    
    printf("请输入用户名：\n");
    scanf("%s",username);

    //登录第一次交互，只填充User结构体的用户名
    strncpy(user->userName, username, sizeof(user->userName)); //2填充用户名
    strcpy(user->salt, "salt");
    strcpy(user->cryptpasswd, "cryptpasswd");
    strcpy(user->pwd, "pwd");
    /* snprintf(request, sizeof(request), "CMD_TYPE_REGISTER_USRNAME:%s", username); */
    
    //填充小火车协议
    train_t t;
    memset(&t, 0, sizeof(t));
    t.len = 1;
    /* t.len = strlen(request); */
    t.type = CMD_TYPE_LOGIN_USRNAME;
    t.user = *user;
    /* strcpy(t.buff, request); */
    //发送小火车
    send(sockfd, &t, 4 + 4 + sizeof(User) + t.len, 0);

    // 接收服务器响应
    receive_response(sockfd, response);
    printf("response = %s\n",response );

    // 盐值检查
    if (strstr(response, "SALT")) {
        sscanf(response, "SALT:%s", salt);
        printf("测试盐值：%s\n", salt);
        printf("-----------\n");

        // 提示用户输入密码
        printf("请输入密码：\n");
        scanf("%s",password);
        printf("输入的密码:%s\n",password);
        /* fgets(password, sizeof(password), stdin); */
        /* password[strcspn(password, "\n")] = 0; // 去除换行符 */

        // 盐值加密
        encrypt_password(password, salt, encrypted_password);
        // 发送加密后的密码
        /* snprintf(request, sizeof(request), "CMD_TYPE_ENCRYPTECODE:%s", encrypted_password); */
        strcpy(user->cryptpasswd, encrypted_password);  // 填充加密密码
        printf("测试:encrypted_password:%s\n",encrypted_password);
        printf("测试: user->cryptpasswd:%s\n",user->cryptpasswd);
        //填充小火车协议
        train_t t;
        memset(&t, 0, sizeof(t));
        t.len = 1;
        /* t.len = strlen(request); */
        t.type = CMD_TYPE_LOGIN_ENCRYTPTEDCODE;
        t.user = *user;
        /* strcpy(t.buff, request); */
        int ret = send(sockfd, &t, 4 + 4 + sizeof(User) + t.len, 0);
        printf("密码send ret = %d\n",ret);

        // 接收服务器最终响应
        receive_response(sockfd, response);
        if(strstr(response, "MSG_TYPE_LOGINOK")) {
            memset(user, 0, sizeof(*user));
            printf("333\n");
            recvUser(sockfd,user);
            printf("444\n");
            return 1;
        }else {
            printf("登录失败，错误代码：%s\n", response);
            return 0;
        }
    }else{
        printf("client:该用户不存在。\n");
        return 0;
    }
}
