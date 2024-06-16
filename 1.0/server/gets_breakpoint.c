#include "thread_pool.h"

int sendn(int sockfd, const void* buff, int len) {
    // 确保将长度为len的数据全部发送完毕
    int left = len;
    const char* pbuf = (char*)buff;

    int ret = 0;
    while (left > 0) {
        ret = send(sockfd, pbuf, left, 0);
        ERROR_CHECK(ret, -1, "send");

        pbuf += ret;
        left -= ret;
    }

    return len - left;
}

void puts_breakpoint(task_t* task) {
    
    // 参数检验，确保task不为NULL
    assert(task);

    
}
