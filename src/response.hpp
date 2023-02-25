#ifndef Response_H
#define Response_H
#include<string>
#include<cstring>
#include<iostream>
#include<ctime>
#include<climits>
class Response{
    public:
    std::string response;
    std::string status_code;
    std::string date;
    std::string last_modified;
    std::string expire_time;
    //std::string etag;
    bool nocache;
    bool is_private;
    bool must_revalidation;
    bool nostore;
    
    int maxstale;
    int maxage;
    
    Response() = default;
    Response(std::string resp): response(resp),status_code(""),date(""),last_modified(""),nocache(false),
    is_private(false),must_revalidation(false),nostore(false),
    maxstale(INT_MAX),maxage(INT_MAX),expire_time(""){
        parse_response();
    };
    ~Response(){};

    void parse_response();
    time_t parse_time(std::string &origin_time);
};




#endif