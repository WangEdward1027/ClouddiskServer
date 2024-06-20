#include "thread_pool.h"

int addFileEntry(FileEntry* fileEntry) {
    MYSQL *conn = create_db_connection();
    
    // 验证ownerId是否存在
    char query[256];
    snprintf(query, sizeof(query), "SELECT id FROM users WHERE id = %d", fileEntry->ownerId);
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
