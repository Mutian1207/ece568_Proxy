#ifndef Proxy_H
#define Proxy_H
#include <cstring>
#include <iostream>
#include"response.hpp"
#include "request.hpp"
#include "cache.hpp"

class Proxy {
 public:
  const char * port;
  Cache cache;
  
  Proxy() = default;
  //destructor
  // ~Proxy() noexcept;
  Proxy(const char * port,const char * capacity);
  //copy constructor
  Proxy(Proxy & rhs);
  //move assignment operator
  Proxy & operator=(Proxy && rhs) noexcept;
  //copy assignment operator
  Proxy & operator=(Proxy & rhs);
  //move constructor
  Proxy(Proxy && rhs) noexcept;

  //start service
  void start_proxy();

  //work on request
  void handle_request(int client_fd, int server_fd, int unique_id, std::string client_ip);
  //handle connect reuqest
  void Connect(int client_fd, int server_fd, int tohost_fd, Request & resquest);
  //hadnle get request;
  void Get(int client_fd, int server_fd, int tohost_fd, Request parsed_request);
  //handle post request;
  void Post(int client_fd, int server_fd, int tohost_fd, Request parsed_request);

  //check 400 bad request
  bool check_400_error(Request parsed_request);
  //check 502 bad gateway
  bool check_502_error(Response parsed_response);
  
  time_t parse_time(std::string &origin_time);
  void revalidate(int client_fd,int server_fd,int tohost_fd,Request& parsed_request,time_t &parsed_date, Response &res, std::string &uri);
  void add_Cache(Response &cache_response, std::string &uri,int unique_id);
  Response fetch_response(int client_fd,int server_fd,int tohost_fd,Request &parsed_request);

};
#endif