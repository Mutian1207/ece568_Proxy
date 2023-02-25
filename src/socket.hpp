#include<iostream>
#include<cstring>
#include<sys/socket.h>
#include<netdb.h>
#include<unistd.h>
#include<arpa/inet.h>
#include<vector>
#ifndef Socket_H
#define Socket_H
class Socket{
    public:
    static int init_server(const char *port);
    static int build_client(const char *hostname,const char *port);
    static int server_accept(int socket_fd,std::string *ip);
    static std::string get_ip(int socket_fd);
    static int get_port(int socket_fd);
};

#endif