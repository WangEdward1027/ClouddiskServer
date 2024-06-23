#include "thread_pool.h"
#include <openssl/rand.h>

// 生成随机Token的函数
void generateToken(char* token, size_t length) {
    static const char charset[] = "abcdefghijklmnopqrstuvwxyzABCDEFGHIJKLMNOPQRSTUVWXYZ0123456789";
    if (length) {
        if (RAND_bytes((unsigned char *)token, length) == 1) {
            for (size_t i = 0; i < length; i++) {
                token[i] = charset[token[i] % (sizeof(charset) - 1)];
            }
            token[length] = '\0';
        } else {
            fprintf(stderr, "Error generating random bytes\n");
            exit(EXIT_FAILURE);
        }
    }
}

// 验证用户并生成Token的函数
Token_t* authenticateUser(char* userName, char* password) {
    // 在这里添加用户验证逻辑
    // 假设用户验证成功，生成Token
    Token_t *token = (Token_t *)malloc(sizeof(Token_t));
    if (token == NULL) {
        fprintf(stderr, "Out of memory.\n");
        return NULL;
    }

    strncpy(token->userName, userName, sizeof(token->userName) - 1);
    token->userName[sizeof(token->userName) - 1] = '\0';
    generateToken(token->Token, 127); // 生成长度为127的Token
    token->Token[127] = '\0';

    // 将Token写入数据库
    if (addToken(token) != 0) {
        free(token);
        return NULL;
    }

    return token;
}

//按用户名查询对应Token值
Token_t* selectTokenByUserName(char* userName) {
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    MYSQL_BIND result[2];
    Token_t *token = NULL;
    const char *sql = "SELECT userName, Token FROM Tokens WHERE userName = ?";

    // 获取数据库连接
    conn = create_db_connection();
    if (conn == NULL) {
        fprintf(stderr, "Database connection failed.\n");
        return NULL;
    }

    // 初始化语句句柄
    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init() out of memory\n");
        mysql_close(conn);
        return NULL;
    }

    // 准备SQL语句
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return NULL;
    }

    // 清除绑定
    memset(bind, 0, sizeof(bind));

    // 绑定输入参数
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)userName;
    bind[0].buffer_length = strlen(userName);
    bind[0].is_null = 0;
    bind[0].length = 0;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return NULL;
    }

    // 执行查询
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return NULL;
    }

    // 清除结果绑定
    memset(result, 0, sizeof(result));

    // 绑定结果
    token = (Token_t *)malloc(sizeof(Token_t));
    if (token == NULL) {
        fprintf(stderr, "Out of memory.\n");
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return NULL;
    }

    result[0].buffer_type = MYSQL_TYPE_STRING;
    result[0].buffer = token->userName;
    result[0].buffer_length = sizeof(token->userName);

    result[1].buffer_type = MYSQL_TYPE_STRING;
    result[1].buffer = token->Token;
    result[1].buffer_length = sizeof(token->Token);

    if (mysql_stmt_bind_result(stmt, result)) {
        fprintf(stderr, "mysql_stmt_bind_result() failed: %s\n", mysql_stmt_error(stmt));
        free(token);
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return NULL;
    }

    // 获取结果集
    if (mysql_stmt_fetch(stmt) == 0) {
        // 成功获取到数据
    } else {
        // 没有找到数据
        free(token);
        token = NULL;
    }

    // 释放资源
    mysql_stmt_close(stmt);
    mysql_close(conn);

    return token;
}
//增加Token值
int addToken(Token_t *token) {
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    const char *sql = "INSERT INTO Tokens (userName, Token) VALUES (?, ?)";

    // 获取数据库连接
    conn = create_db_connection();
    if (conn == NULL) {
        fprintf(stderr, "Database connection failed.\n");
        return -1;
    }

    // 初始化语句句柄
    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init() out of memory\n");
        mysql_close(conn);
        return -1;
    }

    // 准备SQL语句
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return -1;
    }

    // 清除绑定
    memset(bind, 0, sizeof(bind));

    // 绑定输入参数
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)token->userName;
    bind[0].buffer_length = strlen(token->userName);
    bind[0].is_null = 0;
    bind[0].length = 0;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char *)token->Token;
    bind[1].buffer_length = strlen(token->Token);
    bind[1].is_null = 0;
    bind[1].length = 0;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return -1;
    }

    // 执行插入
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return -1;
    }

    // 释放资源
    mysql_stmt_close(stmt);
    mysql_close(conn);

    return 0; // 成功返回0
}
//根据用户名修改已有Token值
int updateTokenByUserName(char* userName, char* newToken) {
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[2];
    const char *sql = "UPDATE Tokens SET Token = ? WHERE userName = ?";

    // 获取数据库连接
    conn = create_db_connection();
    if (conn == NULL) {
        fprintf(stderr, "Database connection failed.\n");
        return -1;
    }

    // 初始化语句句柄
    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init() out of memory\n");
        mysql_close(conn);
        return -1;
    }

    // 准备SQL语句
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return -1;
    }

    // 清除绑定
    memset(bind, 0, sizeof(bind));

    // 绑定输入参数
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)newToken;
    bind[0].buffer_length = strlen(newToken);
    bind[0].is_null = 0;
    bind[0].length = 0;

    bind[1].buffer_type = MYSQL_TYPE_STRING;
    bind[1].buffer = (char *)userName;
    bind[1].buffer_length = strlen(userName);
    bind[1].is_null = 0;
    bind[1].length = 0;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return -1;
    }

    // 执行更新
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return -1;
    }

    // 释放资源
    mysql_stmt_close(stmt);
    mysql_close(conn);

    return 0; // 成功返回0
}
//根据用户名删除对应Token
int deleteTokenByUserName(char* userName) {
    MYSQL *conn;
    MYSQL_STMT *stmt;
    MYSQL_BIND bind[1];
    const char *sql = "DELETE FROM Tokens WHERE userName = ?";

    // 获取数据库连接
    conn = create_db_connection();
    if (conn == NULL) {
        fprintf(stderr, "Database connection failed.\n");
        return -1;
    }

    // 初始化语句句柄
    stmt = mysql_stmt_init(conn);
    if (!stmt) {
        fprintf(stderr, "mysql_stmt_init() out of memory\n");
        mysql_close(conn);
        return -1;
    }

    // 准备SQL语句
    if (mysql_stmt_prepare(stmt, sql, strlen(sql))) {
        fprintf(stderr, "mysql_stmt_prepare() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return -1;
    }

    // 清除绑定
    memset(bind, 0, sizeof(bind));

    // 绑定输入参数
    bind[0].buffer_type = MYSQL_TYPE_STRING;
    bind[0].buffer = (char *)userName;
    bind[0].buffer_length = strlen(userName);
    bind[0].is_null = 0;
    bind[0].length = 0;

    if (mysql_stmt_bind_param(stmt, bind)) {
        fprintf(stderr, "mysql_stmt_bind_param() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return -1;
    }

    // 执行删除
    if (mysql_stmt_execute(stmt)) {
        fprintf(stderr, "mysql_stmt_execute() failed: %s\n", mysql_stmt_error(stmt));
        mysql_stmt_close(stmt);
        mysql_close(conn);
        return -1;
    }

    // 释放资源
    mysql_stmt_close(stmt);
    mysql_close(conn);

    return 0; // 成功返回0
}
