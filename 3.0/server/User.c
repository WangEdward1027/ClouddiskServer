#include "thread_pool.h"



int addUser(User* user) {
    MYSQL *conn = create_db_connection();
    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO Users (userName, salt, cryptpasswd, pwd)
             VALUES ('%s', '%s', '%s', '%s')",
             user->userName, user->salt, user->cryptpasswd, user->pwd);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "addUser() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}
