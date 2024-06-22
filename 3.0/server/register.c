#include"thread_pool.h"
#include <openssl/rand.h>

#define SALT_LENGTH 16


void generateSalt(char* salt,size_t length){
    RAND_bytes((unsigned char*)salt,length);
    for(size_t i=0;i<length;i++){
        snprintf(salt+(i*2),3,"%02x",(unsigned char)salt[i]);
    }
    salt[length*2]='\0';
}

//用户注册：先收用户名
void userRegister1(task_t* task){
    char username[64];
    
    sscanf(task->data,"CMD_TYPE_REGISTER_USERNAME:%s",username);
    
    if(selectUserByUserName(task->user->userName)){
        snprintf(task->data,sizeof(task->data),"USER_EXISTS");
    }else{
        char salt[SALT_LENGTH*2+1];
        generateSalt(salt,SALT_LENGTH);
        
        strncpy(task->user->userName,username,sizeof(task->user->userName));

        snprintf(task->data,sizeof(task->data),"SALT:%s",salt);
        //将盐值保存以便后续使用
        strncpy(task->user->salt,salt,sizeof(task->user->salt));
    }
    send(task->peerfd,task->data,strlen(task->data),0);
}

//用户注册：接收密码
void userRegister2(task_t* task){
    char encrypted_password[65];
    sscanf(task->data,"CMD_TYPE_ENCRYPTECODE:%s",encrypted_password);
    
    if(addUser(task->user)==0){
        snprintf(task->data,sizeof(task->data),"MSG_TYPE_REGISTEROK");
        task->user=selectUserByUserName(task->user->userName);
        //添加虚拟文件根目录
       FileEntry rootFileEntry;
       rootFileEntry.id=1;
       rootFileEntry.parentId=0;
       strncpy(rootFileEntry.fileName,task->user->pwd,sizeof(rootFileEntry.fileName));
       rootFileEntry.ownerId=task->user->id;
       rootFileEntry.fileType=0;

       addFileEntry(&rootFileEntry);
    }else{
        snprintf(task->data,sizeof(task->data),"REGISTER_FAILED");
    }
    send(task->peerfd,task->data,strlen(task->data),0);
}

