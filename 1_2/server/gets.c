#include "thread_pool.h"

//下载:服务器根据客户端发来的文件名，把对应的文件发给客户端
void getsCommand(task_t* task){
    //参数校验
    assert(task);

    //读取本地文件
    char filename[20] = {0};
    strcpy(filename, task->data);
    printf("server gets.c filename: %s\n", filename);
    
    //打开文件
    int fd = open(filename, O_RDWR);
    if(fd == -1){
        error(1, errno, "open gets file");
    }
    
    //获取peerfd
    int peerfd = task->peerfd;
    
    //1.发送文件名的长度
    printf("filename的大小:%ld\n", strlen(filename));
    int len = strlen(filename);
    printf("len = %d\n",len);
    send(peerfd, &len, sizeof(len), 0);

    //2.发送文件名
    printf("filename:%s\n", filename);
    send(peerfd, filename, strlen(filename), 0);
     
    //3.1 获取文件的长度
    struct stat st;
    memset(&st, 0, sizeof(st));
    fstat(fd, &st);
    printf("文件%s的大小:%ld\n",filename, st.st_size);
    
    //3.2 发送文件的长度a
    int filelen = st.st_size;
    send(peerfd, &filelen, sizeof(filelen), 0);

    //4.发送文件内容
    char buff[1000] = {0};
    len = read(fd, buff, st.st_size);
    printf("测试: 发送缓冲区内容: buff: %s \n", buff);
    printf("测试:read的返回值: %d\n",len);
    send(peerfd, buff, len, 0);

    /* while(1){ */
    /*     int ret = read(fd, t.buff, sizeof(t.buff)); */
    /*     if(ret != 1000){ */
    /*         printf("read ret: %d\n", ret); */
    /*     } */
    /*     if(ret == 0){ */
    /*         break; // 文件已经读取完毕 */
    /*     } */
    /*     t.len = ret; */
    /*     ret = sendn(peerfd, &t, 8 + t.len); */
    /*     if(ret == -1){ */
    /*         break; */
    /*     } */
    /*     if(ret != 1008){ */
    /*         printf("send ret:%d\n", ret); */
    /*     } */
    /* } */
    


    printf("send file over.\n");
    close(fd);
}
