#ifndef HTTPCONNECTION_H
#define HTTPCONNECTION_H

#include <sys/epoll.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <signal.h>
#include <sys/types.h>
#include <fcntl.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <sys/stat.h>
#include <sys/mman.h>
#include <stdarg.h>
#include <errno.h>
#include "locker.h"
#include <sys/uio.h>

class http_conn {
public:

    static int m_epollfd; // 所有的socket上的事件都被注册到同一个epollfd
    static int m_user_count; // 统计用户的数量
    static const int READ_BUFFER_SIZE = 2048; // 读缓冲区大小
    static const int WRITE_BUFFER_SIZE = 1024; // 写缓冲区大小

    // HTTP请求方法，但是我们只支持GET
    enum METHOD { GET = 0, POST, HEAD, PUT, DELETE, TRACE, OPTIONS, CONNECT};

    /* 
        解析客户端请求时，主状态机的状态
        CHECK_STATE_REQUESTLINE：当前正在分析请求行
        CHECK_STATE_HEADER：当前正在分析头部字段
        CHECK_STATE_CONTENT：当前正在解析请求体 
    */
    enum CHECK_STATE { CHECK_STATE_REQUESTLINE = 0, CHECK_STATE_HEADER, CHECK_STATE_CONTENT};

    // 从状态机的三种可能状态，即行的读取状态，分别表示
    // 1.读取到一个完整的行 2.行出错 3.行数据尚且不完整
    enum LINE_STATUS { LINE_OK = 0, LINE_BAD, LINE_OPEN};

    /* 
        服务器处理HTTP请求的可能结果，报文解析的结果
        NO_REQUEST          :   请求不完整，需要继续读取客户端数据
        GET_REQUEST         :   表示获得了一个完整的客户请求
        BAD_REQUEST         :   表示客户端请求语法错误
        NO_RESOURCE         :   表示服务器没有资源
        FORBIDDEN_REQUEST   :   表示客户端对资源没有足够的访问权限
        FILE_REQUEST        :   文件请求，获取文件成功
        INTERNAL_ERROR      :   表示服务器内部错误
        CLOSED_CONNECTION   :   表示客户端已经关闭连接了 
    */
    enum HTTP_CODE {NO_REQUEST, GET_REQUEST, BAD_REQUEST, NO_RESOURCE, FORBIDDEN_REQUEST, FILE_REQUEST, INTERNAL_ERROR, CLOSED_CONNECTION};

    http_conn() {}
    ~http_conn() {}

    void process(); // 处理客户端请求
    void init(int sockfd, const sockaddr_in &addr); // 初始化新接收的连接
    void close_conn(); // 关闭连接
    bool read(); // 非阻塞的读
    bool write(); // 非阻塞的写



private:
    int m_sockfd; // 该HTTP连接的socket
    sockaddr_in m_address; // 通信的socket地址
    char m_read_buf[READ_BUFFER_SIZE]; // 读缓冲区
    int m_read_index; // 标识读缓冲区中以及读入的客户端数据的最后一个字节的下一个位置

    HTTP_CODE process_read(); //解析HTTP请求
    HTTP_CODE parse_request_line(char *text); //解析请求首行
    HTTP_CODE parse_headers(char *text); //解析请求头部
    HTTP_CODE parse_content(char *text); //解析请求内容
    LINE_STATUS parse_line();

};



#endif