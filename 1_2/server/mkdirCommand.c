/*
 *@author LSQ created
 *
 *
 *
 */

#include "thread_pool.h"

#define MODE 0777

void mkdirCommand(task_t * task){
    
    char message1[128] = "Create dir Sucessful!";
    char message2[128] = "Create dir Failed!";
    printf("Execute mkdir command.");
    
    train_t t;
    memset(&t,0,sizeof(t));//初始化

    if(mkdir( task->data , MODE ) ==-1){
        strcpy(t.buff,message2);
        t.len=strlen(t.buff);
        sendn(task->peerfd,&t,t.len+4);
        syslog(LOG_WARNING,"User execute MKDIR command: Failed.");
    } 
    else{
        strcpy(t.buff,message1);
        t.len=strlen(t.buff);
        sendn(task->peerfd,&t,t.len+4);
        syslog(LOG_INFO,"User execute MKDIR command: Successful.");
    }
}
