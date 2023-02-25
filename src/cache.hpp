#ifndef __CACHE_HPP__
#define __CACHE_HPP__

#include <iostream>
#include <unordered_map>

#include "response.hpp"
#include<list>
using namespace std;

class Cache {
 public:
  int capacity;
  int load;
  unordered_map<string, Response> responseCache;
  list<string> LRU;

  Cache() = default;

  ~Cache(){};
  
  void add(const string & uri, Response & res);
  bool contains(const string & uri);
  Response get(const string & uri);
  void drop(const string &uri);
  void LRU_popEnd();
  void LRU_addtoFront(const string &uri);
  void LRU_delete(const string &uri);
  void LRU_movetoFront(const string &uri);
};

#endif