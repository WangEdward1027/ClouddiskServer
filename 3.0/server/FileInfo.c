#include "thread_pool.h"

int addFileInfo(FileInfo* fileInfo) {
    MYSQL *conn = create_db_connection();
    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO fileinfo (md5, filename) VALUES ('%s', '%s')",
             fileInfo->md5, fileInfo->fileName);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "addFileInfo() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}

FileInfo* selectFileInfo(char* md5, int md5Len) {
    MYSQL *conn = create_db_connection();
    char query[256];
    snprintf(query, sizeof(query), "SELECT * FROM fileinfo WHERE md5 = '%.*s'", md5Len, md5);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "selectFileInfo() failed: %s\n", mysql_error(conn));
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
    FileInfo *fileInfo = NULL;
    if ((row = mysql_fetch_row(res))) {
        fileInfo = (FileInfo *)malloc(sizeof(FileInfo));
        strcpy(fileInfo->md5, row[0]);
        strcpy(fileInfo->fileName, row[1]);
    }

    mysql_free_result(res);
    mysql_close(conn);
    return fileInfo;
}

