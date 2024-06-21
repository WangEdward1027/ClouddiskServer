#include "thread_pool.h"

int addFileEntry(FileEntry* fileEntry) {
    MYSQL *conn = create_db_connection();
    
    // 验证ownerId是否存在
    char query[256];
    snprintf(query, sizeof(query), "SELECT id FROM user WHERE id = %d", fileEntry->ownerId);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "addFileEntry() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    MYSQL_RES *res = mysql_store_result(conn);
    if (res == NULL || mysql_num_rows(res) == 0) {
        fprintf(stderr, "addFileEntry() failed: ownerId does not exist\n");
        mysql_free_result(res);
        mysql_close(conn);
        return -1;
    }
    mysql_free_result(res);
    
    // 验证md5是否存在
    snprintf(query, sizeof(query), "SELECT md5 FROM fileinfo WHERE md5 = '%s'", fileEntry->md5);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "addFileEntry() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    res = mysql_store_result(conn);
    if (res == NULL || mysql_num_rows(res) == 0) {
        fprintf(stderr, "addFileEntry() failed: md5 does not exist\n");
        mysql_free_result(res);
        mysql_close(conn);
        return -1;
    }
    mysql_free_result(res);
    
    // 插入FileEntry
    snprintf(query, sizeof(query),
             "INSERT INTO fileentry (parentId, fileName, ownerId, md5, fileSize, fileType) VALUES (%d, '%s', %d, '%s', %d, %d)",
             fileEntry->parentId, fileEntry->fileName, fileEntry->ownerId, fileEntry->md5, fileEntry->fileSize, fileEntry->fileType);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "addFileEntry() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}

FileEntry* selectFileEntry(int id) {
    MYSQL *conn = create_db_connection();
    char query[128];
    snprintf(query, sizeof(query), "SELECT * FROM fileentry WHERE id = %d", id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "selectFileEntry() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (res == NULL) {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    MYSQL_ROW row;
    FileEntry *fileEntry = NULL;
    if ((row = mysql_fetch_row(res))) {
        fileEntry = (FileEntry *)malloc(sizeof(FileEntry));
        fileEntry->id = atoi(row[0]);
        fileEntry->parentId = atoi(row[1]);
        strcpy(fileEntry->fileName, row[2]);
        fileEntry->ownerId = atoi(row[3]);
        strcpy(fileEntry->md5, row[4]);
        fileEntry->fileSize = atoi(row[5]);
        fileEntry->fileType = atoi(row[6]);
    }

    mysql_free_result(res);
    mysql_close(conn);
    return fileEntry;
}

int updateFileEntry(FileEntry* fileEntry) {
    MYSQL *conn = create_db_connection();
    
    // 验证ownerId是否存在
    char query[256];
    snprintf(query, sizeof(query), "SELECT id FROM users WHERE id = %d", fileEntry->ownerId);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "updateFileEntry() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    MYSQL_RES *res = mysql_store_result(conn);
    if (res == NULL || mysql_num_rows(res) == 0) {
        fprintf(stderr, "updateFileEntry() failed: ownerId does not exist\n");
        mysql_free_result(res);
        mysql_close(conn);
        return -1;
    }
    mysql_free_result(res);
    
    // 验证md5是否存在
    snprintf(query, sizeof(query), "SELECT md5 FROM fileinfo WHERE md5 = '%s'", fileEntry->md5);
    if (mysql_query(conn, query)) {
        fprintf(stderr, "updateFileEntry() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }
    res = mysql_store_result(conn);
    if (res == NULL || mysql_num_rows(res) == 0) {
        fprintf(stderr, "updateFileEntry() failed: md5 does not exist\n");
        mysql_free_result(res);
        mysql_close(conn);
        return -1;
    }
    mysql_free_result(res);
    
    // 更新FileEntry
    snprintf(query, sizeof(query),
             "UPDATE fileentry SET parent_id = %d, filename = '%s', owner_id = %d, md5 = '%s', filesize = %d, type = %d WHERE id = %d",
             fileEntry->parentId, fileEntry->fileName, fileEntry->ownerId, fileEntry->md5, fileEntry->fileSize, fileEntry->fileType, fileEntry->id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "updateFileEntry() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}

int deleteFileEntry(int id) {
    MYSQL *conn = create_db_connection();
    char query[128];
    snprintf(query, sizeof(query), "DELETE FROM fileentry WHERE id = %d", id);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "deleteFileEntry() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}

// 根据ownerId查询FileEntry
FileEntry* selectFileEntryByOwnerId(int ownerId, int* entryCount) {
    MYSQL *conn = create_db_connection();
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM fileentry WHERE owner_id = %d", ownerId);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "selectFileEntryByOwnerId() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        *entryCount = 0;
        return NULL;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (res == NULL) {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        *entryCount = 0;
        return NULL;
    }

    *entryCount = mysql_num_rows(res);
    if (*entryCount == 0) {
        mysql_free_result(res);
        mysql_close(conn);
        return NULL;
    }

    FileEntry* entries = (FileEntry*)malloc(sizeof(FileEntry) * (*entryCount));
    MYSQL_ROW row;
    int i = 0;
    while ((row = mysql_fetch_row(res))) {
        entries[i].id = atoi(row[0]);
        entries[i].parentId = atoi(row[1]);
        strcpy(entries[i].fileName, row[2]);
        entries[i].ownerId = atoi(row[3]);
        strcpy(entries[i].md5, row[4]);
        entries[i].fileSize = atoi(row[5]);
        entries[i].fileType = atoi(row[6]);
        i++;
    }

    mysql_free_result(res);
    mysql_close(conn);
    return entries;
}

//select FileEntry by fileName;
FileEntry* selectFileEntryByFileName(const char* fileName) {
    MYSQL *conn = create_db_connection();
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM fileentry WHERE filename = '%s'", fileName);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "selectFileEntryByFileName() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (res == NULL) {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }

    MYSQL_ROW row;
    FileEntry *fileEntry = NULL;
    if ((row = mysql_fetch_row(res))) {
        fileEntry = (FileEntry *)malloc(sizeof(FileEntry));
        fileEntry->id = atoi(row[0]);
        fileEntry->parentId = atoi(row[1]);
        strcpy(fileEntry->fileName, row[2]);
        fileEntry->ownerId = atoi(row[3]);
        strcpy(fileEntry->md5, row[4]);
        fileEntry->fileSize = atoi(row[5]);
        fileEntry->fileType = atoi(row[6]);
    }

    mysql_free_result(res);
    mysql_close(conn);
    return fileEntry;
}
// 根据fileName和ownerId查询FileEntry
FileEntry* selectFileEntryByFileNameAndOwnerId(const char* fileName, int ownerId, int* entryCount) {
    MYSQL *conn = create_db_connection();
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM fileentry WHERE filename = '%s' AND owner_id = %d", fileName, ownerId);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "selectFileEntryByFileNameAndOwnerId() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        *entryCount = 0;
        return NULL;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (res == NULL) {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        *entryCount = 0;
        return NULL;
    }

    *entryCount = mysql_num_rows(res);
    if (*entryCount == 0) {
        mysql_free_result(res);
        mysql_close(conn);
        return NULL;
    }

    FileEntry* entries = (FileEntry*)malloc(sizeof(FileEntry) * (*entryCount));
    MYSQL_ROW row;
    int i = 0;
    while ((row = mysql_fetch_row(res))) {
        entries[i].id = atoi(row[0]);
        entries[i].parentId = atoi(row[1]);
        strcpy(entries[i].fileName, row[2]);
        entries[i].ownerId = atoi(row[3]);
        strcpy(entries[i].md5, row[4]);
        entries[i].fileSize = atoi(row[5]);
        entries[i].fileType = atoi(row[6]);
        i++;
    }

    mysql_free_result(res);
    mysql_close(conn);
    return entries;
}

// 根据parentId查询FileEntry
FileEntry* selectFileEntryByparentId(int parentId, int* entryCount) {
    MYSQL *conn = create_db_connection();
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM fileentry WHERE parent_id = %d", parentId);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "selectFileEntryByparentId() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        *entryCount = 0;
        return NULL;
    }

    MYSQL_RES *res = mysql_store_result(conn);
    if (res == NULL) {
        fprintf(stderr, "mysql_store_result() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        *entryCount = 0;
        return NULL;
    }

    *entryCount = mysql_num_rows(res);
    if (*entryCount == 0) {
        mysql_free_result(res);
        mysql_close(conn);
        return NULL;
    }

    FileEntry* entries = (FileEntry*)malloc(sizeof(FileEntry) * (*entryCount));
    MYSQL_ROW row;
    int i = 0;
    while ((row = mysql_fetch_row(res))) {
        entries[i].id = atoi(row[0]);
        entries[i].parentId = atoi(row[1]);
        strcpy(entries[i].fileName, row[2]);
        entries[i].ownerId = atoi(row[3]);
        strcpy(entries[i].md5, row[4]);
        entries[i].fileSize = atoi(row[5]);
        entries[i].fileType = atoi(row[6]);
        i++;
    }

    mysql_free_result(res);
    mysql_close(conn);
    return entries;
}

//---------------------------甘博--------------------------------

FileEntry *getEntriesInDir(int dirId){
    //创建数据库连接
    MYSQL *conn=create_db_connection();
    if(conn==NULL){
        return NULL;
    }

    //SQL查询语句，获取指定父目录ID下所有的条目
    char query[1024];
    snprintf(query,sizeof(query),"SELECT * FROM fileentry WHERE parent_id=%d",dirId);

    //执行SQL语句查询
    if(mysql_query(conn,query)){
        fprintf(stderr,"SELECT ERROR:%s\n",mysql_error(conn));
        mysql_close(conn);
        return NULL;
    }
    
    //获取查询结果
    MYSQL_RES*result=mysql_store_result(conn);
    if(result==NULL){
        fprintf(stderr,"mysql_store_result failed:%s\n",mysql_error(conn));
        mysql_close(conn);
        return NULL; 
    }

    //获取结果集中条目的数量
    int num_rows=mysql_num_rows(result);
    if(num_rows==0){
        mysql_free_result(result);
        mysql_close(conn);
        return NULL;
    }
    
    //为条目数组分配内存
    FileEntry *entries=(FileEntry*)malloc((num_rows+1)*sizeof(FileEntry));
    int index=0;
    MYSQL_ROW row;
    
    //遍历查询结果，填充条目数组
    while((row=mysql_fetch_row(result))){
        entries[index].id=atoi(row[0]);
        entries[index].parentId=atoi(row[1]);
        strncpy(entries[index].fileName,row[2],sizeof(entries[index].fileName)-1);
        entries[index].fileName[sizeof(entries[index].fileName)-1]='\0';
        entries[index].ownerId=atoi(row[3]);
        strncpy(entries[index].md5,row[4],sizeof(entries[index].md5)-1);
        entries[index].md5[sizeof(entries[index].md5)-1]='\0';
        entries[index].fileSize=atoi(row[5]);
        entries[index].fileType=atoi(row[6]);
        index++;
    }

    //添加结束标志
    entries[index].id=0;
    mysql_free_result(result);
    mysql_close(conn);
    return entries;
}

//根据路径查找并返回文件或目录条目
FileEntry *getEntryByPath(const char *path)
{
    //创建数据库连接
    MYSQL *conn =create_db_connection();
    if(conn==NULL){
        return NULL;
    }

    //临时路径存储，防止修改原始路径
    char tempPath[1000];
    strncpy(tempPath,path,sizeof(tempPath)-1);
    tempPath[sizeof(tempPath)-1]='\0';
    
    //使用分隔符“/”解析路径
    char *token =strtok(tempPath,"/");
    int parentId=0;//更目录ID
    FileEntry *entry=NULL;

    //逐级解析路径
    while(token!=NULL){
        char query[1024];
        //构建SQL查询语句，根据当前父亲ID和目录名查找条目
        snprintf(query,sizeof(query),"SELECT * FROM fileentry WHERE parent_Id=%d AND fileName='%s'",parentId,token);

        //执行SQL查询
        if(mysql_query(conn,query)){
            fprintf(stderr,"SELECT ERROR:%s\n",mysql_error(conn));
            mysql_close(conn);
            return NULL;
        }

        //获取查询结果
        MYSQL_RES *result=mysql_store_result(conn);
        if(result==NULL){
            fprintf(stderr,"mysql_store_result() failed:%s\n",mysql_error(conn));
            mysql_close(conn);
            return NULL;
        }

        //获取查询结果的第一行
        MYSQL_ROW row =mysql_fetch_row(result);
        if(row==NULL){
            mysql_free_result(result);
            mysql_close(conn);
            return NULL;
        }

        //填充条目结构体
        entry = (FileEntry *)malloc(sizeof(FileEntry));
        entry->id = atoi(row[0]);
        entry->parentId = atoi(row[1]);
        strncpy(entry->fileName, row[2], sizeof(entry->fileName) - 1);
        entry->fileName[sizeof(entry->fileName) - 1] = '\0';
        entry->ownerId = atoi(row[3]);
        strncpy(entry->md5, row[4], sizeof(entry->md5) - 1);
        entry->md5[sizeof(entry->md5) - 1] = '\0';
        entry->fileSize = atoi(row[5]);
        entry->fileType = atoi(row[6]);

        //更新父ID，继续查找下一层级
        parentId=entry->id;
        token=strtok(NULL,"/");
        mysql_free_result(result);
    }
    mysql_close(conn);
    return entry;
}
