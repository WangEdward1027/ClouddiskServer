#include"client.h"
#include"str_util.h"
#include <openssl/sha.h>

#define BUFFER_SIZE 256

void send_request(int sockfd,char*request){
write(sockfd, request, strlen(request));
}

void receive_response(int sockfd, char *response) {
int len = read(sockfd, response, BUFFER_SIZE - 1);
if (len > 0) {
response[len] = '\0';
}
}

void encrypt_password(const char *password, const char *salt, char *encrypted_password){
char combined[BUFFER_SIZE];
unsigned char hash[SHA256_DIGEST_LENGTH];

//将盐值和密码连接在一起
snprintf(combined,sizeof(combined),"%s%s",password,salt);

//使用SHA-256哈希函数加密
SHA256((unsigned char *)combined,strlen(combined),hash);

//把哈希值转换为十六进制字符串
for(int i=0;i<SHA256_DIGEST_LENGTH;i++){
    snprintf(encrypted_password+(i*2),3,"%02x",hash[i]);
}
encrypted_password[SHA256_DIGEST_LENGTH * 2] = '\0'; // 确保字符串终止符
}

void register_client(int sockfd){
char request[BUFFER_SIZE];
char response[BUFFER_SIZE];
char username[BUFFER_SIZE];
char password[BUFFER_SIZE];
char salt[256];
char encrypted_password[BUFFER_SIZE];

printf("请输出新的用户名：\n");
fgets(username,sizeof(username),stdin);
username[strcspn(username,"\n")]=0;

snprintf(request,sizeof(request),"CMD_TYPE_REGISTER_USRNAME:%s",username);
send_request(sockfd,request);

//接收服务器响应
receive_response(sockfd, response);

//盐值检查
if(strstr(response,"SALT")){
    sscanf(response,"SALT:%s",salt);
    printf("盐值：%s\n",salt);

    //提示用户输入密码
    printf("请输入密码：\n");
    fgets(password,sizeof(password),stdin);
    password[strcspn(password,"\n")]=0;//去除换行符
    
    //盐值加密
    encrypt_password(password, salt, encrypted_password);
    //发送加密后的密码
    snprintf(request,sizeof(request),"CMD_TYPE_ENCRYPTECODE:%s",encrypted_password);
    send_request(sockfd,request);

    //接收服务器最终响应
    receive_response(sockfd, response);
    if(strstr(response,"MSG_TYPE_REGISTEROK")){
        printf("注册成功\n");
    }else{
        printf("注册失败，错误代码：%s\n",response);
    }
}else{
    printf("用户已存在。\n");
}
}
