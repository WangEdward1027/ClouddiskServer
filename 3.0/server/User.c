#include "thread_pool.h"

//insert into user
int addUser(User* user) {
    MYSQL *conn = create_db_connection();
    char query[256];
    snprintf(query, sizeof(query),
             "INSERT INTO user (username, salt, cryptpasswd, pwd)VALUES ('%s', '%s', '%s', '%s')",
             user->userName, user->salt, user->cryptpasswd, user->pwd);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "addUser() failed: %s\n", mysql_error(conn));
        mysql_close(conn);
        return -1;
    }

    mysql_close(conn);
    return 0;
}

//select * from user where id = ?
User* selectUser(int userId) {
    MYSQL *conn = create_db_connection();
    char query[128];
    snprintf(query, sizeof(query), "SELECT * FROM users WHERE id = %d", userId);

    if (mysql_query(conn, query)) {
        fprintf(stderr, "selectUser() failed: %s\n", mysql_error(conn));
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
    User *user = NULL;
    if ((row = mysql_fetch_row(res))) {
        user = (User *)malloc(sizeof(User));
        user->id = atoi(row[0]);
        strcpy(user->userName, row[1]);
        strcpy(user->salt, row[2]);
        strcpy(user->cryptpasswd, row[3]);
        strcpy(user->pwd, row[4]);
    }

    mysql_free_result(res);
    mysql_close(conn);
    return user;
}


