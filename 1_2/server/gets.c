#include "thread_pool.h"

//下载:接收从服务器发来的文件
void getsCommand(task_t* task){
    //参数校验
    assert(task);

    //读取本地文件
    char filename[128];
    strcpy(filename, task->data);
    int fd = open(filename, O_RDWR);

    train_t t;
    memset(&t, 0, sizeof(t));
    int peerfd = task->peerfd;
    
    //先发送文件名
    t.len = strlen(filename);
    strcpy(t.buff, filename);
    send(peerfd, &t, 4 + t.len, 0); //将文件名长度 + 文件名,发送给对方
    
    //其次发送文件长度
    struct stat st;
    memset(&st, 0, sizeof(st));
    fstat(fd, &st);
    printf("filelength:%ld\n",st.st_size);
    printf("sizeof(st.st_size):%ld\n", sizeof(st.st_size));
    send(peerfd, &st.st_size, sizeof(st.st_size), 0);

    //最后发送文件的内容
    while(1){
        memset(&t, 0, sizeof(t));
        int ret = read(fd, t.buff, sizeof(t.buff));
        if(ret != 1000){
            printf("read ret: %d\n", ret);
        }
        if(ret == 0){
            break; // 文件已经读取完毕
        }
        t.len = ret;
        ret = sendn(peerfd, &t, 4 + t.len);
        if(ret == -1){
            break;
        }
        if(ret != 1004){
            printf("send ret:%d\n", ret);
        }
    }
    printf("send file over.\n");
    close(fd);
}
