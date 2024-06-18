#include "client.h"

int parseCommand(const char* buff, int len, train_t* pt){
    pt->len = strlen(pt->buff);
    
    //把buff里的第一个命令分词，然后判断CmdTpe的类型,填入train_t中
    char* tempbuff =(char *)calloc(len + 1, sizeof(char));
    strcpy(tempbuff, buff);     //buff是const
    char * token = strtok(tempbuff, " "); //按照空格进行分词

    if(token == NULL) return -1;  //解析失败
    if(strcmp(token, "pwd")){
        pt->type = CMD_TYPE_PWD;
    }else if(strcmp(token, "ls")){
        pt->type = CMD_TYPE_LS;
    }else if(strcmp(token, "cd")){
        pt->type = CMD_TYPE_CD;
    }else if(strcmp(token, "mkdir")){
        pt->type = CMD_TYPE_MKDIR;
    }else if(strcmp(token, "rmdir")){
        pt->type = CMD_TYPE_RMDIR;
    }else if(strcmp(token, "puts")){
        pt->type = CMD_TYPE_PUTS;
    }else if(strcmp(token, "gets")){
        pt->type = CMD_TYPE_GETS;
    }else if(strcmp(token, "tree")){
        pt->type = CMD_TYPE_TREE;
    }else if(strcmp(token, "usrname")){
        pt->type = CMD_TYPE_USRNAME;
    }else if(strcmp(token, "usrencode")){
        pt->type = CMD_TYPE_USRENCODE;
    }else{
        pt->type = CMD_TYPE_NOTCMD;
    }
    
    //把剩余的字符串放进train_t 的 buff里
    strcpy(pt->buff, strtok(NULL, " "));
    free(tempbuff);
    return 0;
}
