#include "socket.hpp"

int Socket::init_server(const char * port) {
  const char * hostname = NULL;
  int sock_fd;
  struct addrinfo host;
  struct addrinfo * host_info_list;

  memset(&host, 0, sizeof(host));
  host.ai_family = AF_UNSPEC;
  host.ai_socktype = SOCK_STREAM;
  host.ai_flags = AI_PASSIVE;
  int status = 0;
  status = getaddrinfo(hostname, port, &host, &host_info_list);
  if (status != 0) {
    std::cerr << "ERROR: cannot get address info for host" << std::endl;
    std::cerr << " (hostname: " << hostname << " "
              << ", port: " << port << ")" << std::endl;
    return -1;
  }
  if (strcmp(port, "") == 0) {
    struct sockaddr_in * addr_in = (struct sockaddr_in *)(host_info_list->ai_addr);
    addr_in->sin_port = 0;
  }
  sock_fd = socket(host_info_list->ai_family,
                   host_info_list->ai_socktype,
                   host_info_list->ai_protocol);
  if (sock_fd == -1) {
    std::cerr << "ERROR: cannot create socket" << std::endl;
    std::cerr << " (hostname: " << hostname << " "
              << ", port: " << port << ")" << std::endl;
    return -1;
  }
  int yes = 1;
  setsockopt(sock_fd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(int));
  if (bind(sock_fd, host_info_list->ai_addr, host_info_list->ai_addrlen) == -1) {
    std::cerr << "ERROR: cannot bind socket" << std::endl;
    std::cerr << " (hostname: " << hostname << " "
              << ", port: " << port << ")" << std::endl;
    return -1;
  }
  if (listen(sock_fd, 100) == -1) {
    std::cerr << "ERROR: cannot listen on socket" << std::endl;
    std::cerr << " (hostname: " << hostname << " "
              << ", port: " << port << ")" << std::endl;
    return -1;
  }
  freeaddrinfo(host_info_list);
  return sock_fd;
}

int Socket::build_client(const char * hostname, const char * port) {
  struct addrinfo host;
  struct addrinfo * host_info_list;
  int sock_fd;
  // std::cout<<"const char *hostname:"<<hostname<<std::endl;
  memset(&host, 0, sizeof(host));
  host.ai_family = AF_UNSPEC;
  host.ai_socktype = SOCK_STREAM;
  if (getaddrinfo(hostname, port, &host, &host_info_list) != 0) {
    std::cerr << "ERROR: cannot get address info for host" << std::endl;
    std::cerr << " (hostname: " << hostname << " "
              << ", port: " << port << ")" << std::endl;
    return -1;
  }
  sock_fd = socket(host_info_list->ai_family,
                   host_info_list->ai_socktype,
                   host_info_list->ai_protocol);
  if (sock_fd == -1) {
    std::cerr << "ERROR: cannot create socket" << std::endl;
    std::cerr << " (hostname: " << hostname << " "
              << ", port: " << port << ")" << std::endl;
    return -1;
  }
  //  std::cout<<"connecting to hostname:"<<hostname<<"on port:"<<port<<"..."<<std::endl;
  if (connect(sock_fd, host_info_list->ai_addr, host_info_list->ai_addrlen) == -1) {
    std::cerr << "ERROR: cannot connect to socket" << std::endl;
    std::cerr << " (hostname: " << hostname << " "
              << ", port: " << port << ")" << std::endl;
    return -1;
  }
  freeaddrinfo(host_info_list);
  return sock_fd;
}

int Socket::server_accept(int socket_fd, std::string * ip) {
  struct sockaddr addr;
  socklen_t addrlen = sizeof(addr);
  int client_fd = accept(socket_fd, &addr, &addrlen);
  if (client_fd == -1) {
    std::cerr << "Error: cannot accept connection" << std::endl;
    return -1;
  }

  *ip = inet_ntoa(((struct sockaddr_in *)&addr)->sin_addr);
  return client_fd;
}

std::string Socket::get_ip(int socket_fd) {
  struct sockaddr_in sock_addr;
  socklen_t len = sizeof(sock_addr);
  if (getsockname(socket_fd, (struct sockaddr *)&sock_addr, &len) == -1) {
    std::cerr << "ERROR: cannot get IP address" << std::endl;
  }
  // const char *inet_ntop(int af, const void *restrict src,
  //                      char *restrict dst, socklen_t size);
  std::string ip;
  ip = inet_ntoa(sock_addr.sin_addr);
  return ip;
}

int Socket::get_port(int socket_fd) {
  struct sockaddr addr;
  socklen_t addrlen = sizeof(addr);
  bzero(&addr, addrlen);
  getsockname(socket_fd, &addr, &addrlen);
  struct sockaddr_in * addr_in = (struct sockaddr_in *)&addr;
  return ntohs(addr_in->sin_port);
}