#include<iostream>
#include<cstring>
#include<boost/asio.hpp>
#include<boost/beast.hpp>
#ifndef Request_H
#define Request_H
class Request{
    public:
    //original request from client
    std::string request;
    std::string method;
    std::string url_target;
    //http protocol version
    //std::string version; 
    std::string host;
    std::string hostname;
    std::string port;
    std::string user_agent;
    
    std::string firstline;
    std::string body;
    std::string sent_time;
    int unique_id;
    Request()=default;
    Request(const char * ori_req);
    ~Request(){};
};
#endif