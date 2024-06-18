#include "thread_pool.h"

// 将对应的枚举类型CMD_TYPE转化成string
// 方便在写日志时将对应的type转换成string写入，更加直观
const char* cmdTypeToString(CmdType type) {
    switch (type) {
        case CMD_TYPE_PWD:
            return "CMD_TYPE_PWD";
        case CMD_TYPE_LS:
            return "CMD_TYPE_LS";
        case CMD_TYPE_CD:
            return "CMD_TYPE_CD";
        case CMD_TYPE_MKDIR:
            return "CMD_TYPE_MKDIR";
        case CMD_TYPE_RMDIR:
            return "CMD_TYPE_RMDIR";
        case CMD_TYPE_PUTS:
            return "CMD_TYPE_PUTS";
        case CMD_TYPE_GETS:
            return "CMD_TYPE_GETS";
        case CMD_TYPE_TREE:
            return "CMD_TYPE_TREE";
        case CMD_TYPE_NOTCMD:
            return "CMD_TYPE_NOTCMD";
        default:
            return "Unknown";
    }
}

void log_login(char* username) {
    
    time_t now = time(NULL); // 记录当前时间
    
    // 打开syslog连接，设置标识符为clouddiskserver，
    // 在日志信息中包含进程ID，如果无法连接到syslog将相关内容写到控制台
    // 将日志信息归类为用户级别，表示与用户相关的信息
    openlog("ClouddiskServer", LOG_PID | LOG_CONS, LOG_USER);
    
    // 向系统log写入一条日志，表示为xxx时:用户xxx 登陆成功！
    syslog(LOG_INFO, "login_time:%.24s user:%s 登陆成功！\n", ctime(&now), username);

    // 关闭Syslog 连接
    closelog();

}

void log_action(char* username, task_t* task) {

    time_t now = time(NULL);

    openlog("ClouddiskServer", LOG_PID | LOG_CONS, LOG_USER);

    syslog(LOG_INFO, "action_time:%.24s user:%s file:%s action:%s", 
           ctime(&now), username, task->data, cmdTypeToString(task->type));

    closelog();
}

void logCommand(task_t* task) {
    FILE* logFile;
    char logLine[256];

    char buff[4096];
    bzero(buff, sizeof(buff));

    // 打开系统日志文件
    logFile = fopen("/var/log/syslog", "r");
    if (logFile == NULL) {
        error(1, errno, "fopen failed");
    }

    // 逐行读取日志文件输出
    while (fgets(logLine, sizeof(logLine), logFile) != NULL) {
        // 检查与项目相关的目录行
        if (strstr(logLine, "ClouddiskServer") != NULL) {
            printf("%s", logLine);
            sprintf(buff, "%s\n", logLine);
        }
    }

    // 发送日志到客户端
    sendn(task->peerfd, buff, sizeof(buff));

    // 关闭日志文件
    fclose(logFile);
}
