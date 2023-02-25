#include "proxy.hpp"

#include <ctime>
#include <iomanip>
#include <iostream>
#include <mutex>
#include <thread>

#include "cache.hpp"
#include "log.cpp"
#include "request.hpp"
#include "response.hpp"
#include "socket.hpp"
using sysclock_t = std::chrono::system_clock;
std::mutex Mutex;
Proxy::Proxy(const char *port,const char *capacity):port(port){
  cache.load = 0;
  cache.capacity = std::stoi(capacity);
}

// copy constructor
Proxy::Proxy(Proxy & rhs) : port(new char[strlen(rhs.port)]) {
  char tmp[strlen(rhs.port) + 1];
  memcpy(tmp, rhs.port, strlen(rhs.port));
  port = tmp;
};
// move assignment operator
Proxy & Proxy::operator=(Proxy && rhs) noexcept {
  if (this != &rhs) {
    std::swap(port, rhs.port);
  }
  return *this;
}
// copy assignment operator
Proxy & Proxy::operator=(Proxy & rhs) {
  if (this != &rhs) {
    char * tmp = new char[strlen(rhs.port)];
    for (auto i = 0; i < strlen(rhs.port); i++) {
      tmp[i] = rhs.port[i];
    }
    delete[] port;
    port = tmp;
  }
  return *this;
};
// move constructor
Proxy::Proxy(Proxy && rhs) noexcept {
  port = nullptr;
  std::swap(rhs.port, port);
}

// start proxy service
void Proxy::start_proxy() {
  int server_fd = Socket::init_server(port);
  int unique_id = 100;
  if (server_fd == -1) {
    // create error -> logfile
    std::cout << "create error" << std::endl;
  }
  std::cout << "build proxy server" << std::endl;

  while (1) {
    std::string client_ip;
    int client_fd = Socket::server_accept(server_fd, &client_ip);
    if (client_fd == -1) {
      // logfile
      continue;
    }

    // concurrency for requests (add thread id unique id later )
    std::thread new_t([this, client_fd, server_fd, unique_id, client_ip]() {
      handle_request(client_fd, server_fd, unique_id, client_ip);
    });
    unique_id++;
    new_t.detach();
  }
  close(server_fd);
}

void Proxy::handle_request(int client_fd,
                           int server_fd,
                           int unique_id,
                           std::string client_ip) {
  // receive request from client
  char req[65535];
  int size_req = recv(client_fd, req, sizeof(req), 0);

  // std::cout<<"The request is "<<std::endl;
  // std::cout<<req<<std::endl;
  
  // handle request (parse request)
  Request parsed_request(req);
  parsed_request.unique_id = unique_id;
  //handle 400 bad request
  if(check_400_error(parsed_request)){
    std::string error_msg = "HTTP/1.1 400 Bad Request\r\n\r\n";
    send(client_fd,error_msg.c_str(),error_msg.size(),0);
    logMessage(std::to_string(unique_id)+": ERROR: 400 bad request");
    return;
  }
 
  // client ip???
  logMessage(std::to_string(unique_id) + ": " + parsed_request.firstline + " from " +
             client_ip + " @ " + parsed_request.sent_time);

  // CONNECT only happens to webpages only do HTTPS
  // thus default port is "443" if empty
  if (parsed_request.method == "CONNECT") {
    if (parsed_request.port.empty()) {
      parsed_request.port = "443";
    }
    int tohost_fd = Socket::build_client(parsed_request.hostname.c_str(),
                                         parsed_request.port.c_str());
    //502 ERROR CODE
    if (tohost_fd == -1) {
      std::string error_msg = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
      send(client_fd,error_msg.c_str(),error_msg.size(),0);
      logMessage(std::to_string(unique_id) + ": Warning: cannot connect to upstream server");
      logMessage(std::to_string(unique_id) + ": ERROR: 502 Bad Gateway");
      return;
    }
    Connect(client_fd, server_fd, tohost_fd, parsed_request);
    close(tohost_fd);
    logMessage(std::to_string(unique_id) + ": Tunnel closed");
  }
  else if (parsed_request.method == "GET") {
    if (parsed_request.port.empty()) {
      parsed_request.port = "80";
    }
    int tohost_fd = Socket::build_client(parsed_request.hostname.c_str(),
                                         parsed_request.port.c_str());
    //502 ERROR CODE
    if (tohost_fd == -1) {
      std::string error_msg = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
      send(client_fd,error_msg.c_str(),error_msg.size(),0);
      logMessage(std::to_string(unique_id) + ": Warning: cannot connect to upstream server");
      logMessage(std::to_string(unique_id) + ": ERROR: 502 Bad Gateway");
      return;
    }
    Get(client_fd, server_fd, tohost_fd, parsed_request);
    close(tohost_fd);
    logMessage(std::to_string(unique_id) + ": Tunnel closed");
  }
  else if (parsed_request.method == "POST") {
    if (parsed_request.port.empty()) {
      parsed_request.port = "80";
    }
    int tohost_fd = Socket::build_client(parsed_request.hostname.c_str(),
                                         parsed_request.port.c_str());
    //502 ERROR CODE
    if (tohost_fd == -1) {
      std::string error_msg = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
      send(client_fd,error_msg.c_str(),error_msg.size(),0);
      logMessage(std::to_string(unique_id) + ": Warning: cannot connect to upstream server");
      logMessage(std::to_string(unique_id) + ": ERROR: 502 Bad Gateway");
      return;
    }
    Post(client_fd, server_fd, tohost_fd, parsed_request);
    close(tohost_fd);
    logMessage(std::to_string(unique_id) + ": Tunnel closed");
  }
  else {
    logMessage(std::to_string(unique_id) + ": WARNING: only handle get post connect");
  }
  close(client_fd);
}

void Proxy::Connect(int client_fd, int server_fd, int tohost_fd, Request & request) {
  // step1 open a socket with the server

  // step2 send an http response of 200 ok back to browser
  std::string msg = "HTTP/1.1 200 OK\r\n\r\n";
  logMessage(std::to_string(request.unique_id) + ": Requesting \"" + request.firstline +
             "\" from " + request.hostname);
  send(client_fd, msg.c_str(), msg.size(), 0);
  logMessage(std::to_string(request.unique_id) + ": Responding \"HTTP/1.1 200 OK\"");
  // step3 Use IO multiplexing (i.e. select()) from both ports (client and server),
  //  simply forwarding messages from one end to another.
  fd_set rfds;
  int maxfd = std::max(client_fd, tohost_fd);
  // I/O using select
  while (1) {
    FD_ZERO(&rfds);
    FD_SET(client_fd, &rfds);
    FD_SET(tohost_fd, &rfds);
    select(maxfd + 1, &rfds, NULL, NULL, NULL);

    char msg[65536];
    if (FD_ISSET(client_fd, &rfds)) {
      int RecvBytes = recv(client_fd, msg, sizeof(msg), MSG_NOSIGNAL);
      if (RecvBytes <= 0) {
        return;
      }
      int SendBytes = send(tohost_fd, msg, RecvBytes, MSG_NOSIGNAL);
      if (SendBytes <= 0) {
        return;
      }
    }
    if (FD_ISSET(tohost_fd, &rfds)) {
      int RecvBytes = recv(tohost_fd, msg, sizeof(msg), MSG_NOSIGNAL);
      if (RecvBytes <= 0) {
        return;
      }
      int SendBytes = send(client_fd, msg, RecvBytes, MSG_NOSIGNAL);
      if (SendBytes <= 0) {
        return;
      }
    }
  }
}

void Proxy::Get(int client_fd, int server_fd, int tohost_fd, Request parsed_request) {
  // 1.  check cache for resource.
  std::string uri = parsed_request.url_target;
  // 2.if found in cache and resource still valid, sent it to client
  // when revalidation?

  // 3.if not in caceh or has expired-> refetch from the server

  // 4.The proxy forwards the GET request to the server.

  // 5.Receive response from server: The proxy receives the response from the server.

  // 6.Cache resource: If the response from the server includes Cache-Control headers that allow caching,
  // the proxy caches the resource for future requests.

  // 7.send response to client from proxy

  if (cache.contains(uri)) {
    Mutex.lock();
    Response res = cache.get(uri);
    Mutex.unlock();
    // ID: in cache, valid
    // send the cache to clinet
    time_t parsed_date = parse_time(res.date);
    std::time_t now = sysclock_t::to_time_t(sysclock_t::now());
    // no cache
    if (res.nocache) {
      // revalidate everytime
      revalidate(client_fd, server_fd, tohost_fd, parsed_request, parsed_date, res, uri);
    }
    if (parsed_date + res.maxage > now) {
      // fresh send directly
      logMessage(std::to_string(parsed_request.unique_id) + ": in cache, valid");
      std::cout << "send from cache" << std::endl;
      std::cout << std::this_thread::get_id() << ": in cache, valid" << std::endl;
      std::cout << "send response" << res.response << std::endl;
      logMessage(std::to_string(parsed_request.unique_id) + ": Responding \"" +
                 res.status_code + "\"");
      send(client_fd, res.response.c_str(), res.response.size(), MSG_NOSIGNAL);
      return;
    }
    else if (now < parsed_date + res.maxage + res.maxstale) {
      //stale
      // revalidate
      logMessage(std::to_string(parsed_request.unique_id) +
                 ": in cache, requires validation");
      revalidate(client_fd, server_fd, tohost_fd, parsed_request, parsed_date, res, uri);
    }
    else {  //invalid
      // replace == cache::add

      logMessage(std::to_string(parsed_request.unique_id) +
                 ": in cache, but expired at " + res.expire_time);

      cache.drop(uri);
      //refetch the response and cache
      logMessage(std::to_string(parsed_request.unique_id) + ": Requesting\"" +
                 parsed_request.firstline + "\" from " + parsed_request.hostname);
      Response parsed_response =
          fetch_response(client_fd, server_fd, tohost_fd, parsed_request);
      //handle response 502 ERROR
      if(check_502_error(parsed_response)){
        std::string error_msg = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
        send(client_fd, error_msg.c_str(), error_msg.size(), MSG_NOSIGNAL);
        logMessage(std::to_string(parsed_request.unique_id) + ": Warning: invalid response" );
        logMessage(std::to_string(parsed_request.unique_id) + ": ERROR: 502 Bad Gateway" );
        return;
      }
      logMessage(std::to_string(parsed_request.unique_id) + ": Received\"" +
                 parsed_response.status_code + "\" from " + parsed_request.hostname);

      if (parsed_response.status_code.find("200 OK") != std::string::npos) {
        add_Cache(parsed_response, uri, parsed_request.unique_id);
      }
      logMessage(std::to_string(parsed_request.unique_id) + ": Responding \"" +
                 parsed_response.status_code + "\"");
      send(client_fd,
           parsed_response.response.c_str(),
           parsed_response.response.size(),
           0);
    }
  }
  else {  // ID: not in cache
    // ID: in cache, valid
    // send the cache to clinet
    logMessage(std::to_string(parsed_request.unique_id) + ": not in cache");
    std::cout << uri << " not in cache" << std::endl;
    logMessage(std::to_string(parsed_request.unique_id) + ": Requesting \"" +
               parsed_request.firstline + "\" from " + parsed_request.hostname);
    Response parsed_response =
        fetch_response(client_fd, server_fd, tohost_fd, parsed_request);
    //handle response 502 ERROR
    if(check_502_error(parsed_response)){
      std::string error_msg = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
      send(client_fd, error_msg.c_str(), error_msg.size(), MSG_NOSIGNAL);
      logMessage(std::to_string(parsed_request.unique_id) + ": Warning: invalid response" );
      logMessage(std::to_string(parsed_request.unique_id) + ": ERROR: 502 Bad Gateway" );
      return;
    }
    logMessage(std::to_string(parsed_request.unique_id) + ": Received \"" +
               parsed_response.status_code + "\" from " + parsed_request.hostname);
    if (parsed_response.status_code.find("200 OK") != std::string::npos) {  // 200 ok
      add_Cache(parsed_response, uri, parsed_request.unique_id);
    }
    logMessage(std::to_string(parsed_request.unique_id) + ": Responding \"" +
               parsed_response.status_code + "\"");
    send(client_fd, parsed_response.response.c_str(), parsed_response.response.size(), 0);
  }
}

void Proxy::Post(int client_fd, int server_fd, int tohost_fd, Request parsed_request) {
  //send request to server
  send(tohost_fd, parsed_request.request.c_str(), parsed_request.request.size(), 0);
  //receive response
  char buffer[65536];
  memset(buffer,0,sizeof(buffer));
  recv(tohost_fd, buffer, sizeof(buffer), 0);
  std::string response(buffer);
  Response parsed_response(response);
  //handle response 502 ERROR
  if(check_502_error(parsed_response)){
    std::string error_msg = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
    send(client_fd, error_msg.c_str(), error_msg.size(), MSG_NOSIGNAL);
    logMessage(std::to_string(parsed_request.unique_id) + ": Warning: invalid response" );
    logMessage(std::to_string(parsed_request.unique_id) + ": ERROR: 502 Bad Gateway" );
    return;
  }
  //no ERROR 
  logMessage(std::to_string(parsed_request.unique_id) + ": Responding \"" +
             parsed_response.status_code + "\"");
  send(client_fd, response.c_str(), response.size(), 0);
};

time_t Proxy::parse_time(std::string & origin_time) {
  struct tm tm;
  memset(&tm,0,sizeof(tm));
  std::istringstream iss(origin_time);
  iss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S");
  time_t time = mktime(&tm);
  return time;
}

void Proxy::revalidate(int client_fd,
                       int server_fd,
                       int tohost_fd,
                       Request & parsed_request,
                       time_t & parsed_date,
                       Response & res,
                       std::string & uri) {
  send(tohost_fd,
       parsed_request.request.c_str(),
       parsed_request.request.size(),
       MSG_NOSIGNAL);
  char response[65536];
  recv(tohost_fd, response, sizeof(response), 0);
  std::string str_response(response);
  std::cout << "response from server:" << str_response << std::endl;
  Response new_response(str_response);

  //handle response 502 ERROR
  if(check_502_error(new_response)){
    std::string error_msg = "HTTP/1.1 502 Bad Gateway\r\n\r\n";
    send(client_fd, error_msg.c_str(), error_msg.size(), MSG_NOSIGNAL);
    logMessage(std::to_string(parsed_request.unique_id) + ": Warning: invalid response" );
    logMessage(std::to_string(parsed_request.unique_id) + ": ERROR: 502 Bad Gateway" );
    return;
  }

  time_t last_modified = parse_time(new_response.last_modified);
  
  //if content unchanged
  if (parsed_date > last_modified) {
    logMessage(std::to_string(parsed_request.unique_id) + ": Responding \"" +
               res.status_code + "\"");
    send(client_fd, res.response.c_str(), res.response.size(), MSG_NOSIGNAL);
    return;
  }
  else {
    cache.drop(uri);
    cache.add(uri,new_response);
    logMessage(std::to_string(parsed_request.unique_id) + ": Responding \"" +
               new_response.status_code + "\"");
    send(client_fd,
         new_response.response.c_str(),
         new_response.response.size(),
         MSG_NOSIGNAL);
    return;
  }
}

Response Proxy::fetch_response(int client_fd,
                               int server_fd,
                               int tohost_fd,
                               Request & parsed_request) {
  //logMessage(std::to_string(parsed_request.unique_id) + ": Requsting " + parsed_request.firstline + " from " + parsed_request.hostname);
  send(tohost_fd,
       parsed_request.request.c_str(),
       parsed_request.request.size(),
       MSG_NOSIGNAL);
  // receive from the original server
  char response[65536];
  recv(tohost_fd, response, sizeof(response), 0);
  std::string str_response(response);
  std::cout << "response from server:" << str_response << std::endl;
  Response cache_response(str_response);
  //logMessage(std::to_string(parsed_request.unique_id) + ": Responding " + cache_response.status_code);
  return cache_response;
}

void Proxy::add_Cache(Response & cache_response, std::string & uri, int unique_id) {
  // to tell if it can be cached
  if (cache_response.is_private) {
    // cannot be cache
    // log : id : not cachable because private//nostore
    logMessage(std::to_string(unique_id) + ": not cacheable because response is private");
    return;
  }
  if (cache_response.nostore) {
    logMessage(std::to_string(unique_id) +
               ": not cacheable because response is no-store");
    return;
  }
  if (cache_response.must_revalidation) {
    if(cache_response.maxstale!=INT_MAX){
      logMessage(std::to_string(unique_id) + ": cached, expires at "+cache_response.expire_time);
    }
    logMessage(std::to_string(unique_id) + ": cached, but requires re-validation");
    logMessage(std::to_string(unique_id) + ": NOTE Cache-Control: must validate");
  } else if (cache_response.nocache) {
    if(cache_response.maxstale!=INT_MAX){
       logMessage(std::to_string(unique_id) + ": cached, expires at "+cache_response.expire_time);
    }
    logMessage(std::to_string(unique_id) + ": cached, but requires re-validation");
    logMessage(std::to_string(unique_id) + ": NOTE Cache-Control: no cache");
  }else{
    logMessage(std::to_string(unique_id) + ": cached, expires at "+cache_response.expire_time);
  }

  if (!cache_response.last_modified.empty()) {
    logMessage(std::to_string(unique_id) + ": NOTE Last Modified: " + cache_response.last_modified);
  }

  std::cout << "add " << uri << " to cache" << std::endl;
  Mutex.lock();
  cache.add(uri, cache_response);
  Mutex.unlock();
}

bool Proxy::check_400_error(Request parsed_request){

 if (parsed_request.firstline.empty() || 
    parsed_request.hostname.empty() ||
    parsed_request.method.empty() ||
    parsed_request.url_target.empty()) {
    return true;
  }

  return false;
}


bool Proxy::check_502_error(Response parsed_response){
  if(parsed_response.status_code.empty()||
    parsed_response.response.empty()){
      return true;
    }
  return false;
}
  