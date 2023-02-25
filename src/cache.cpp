#include "cache.hpp"

void Cache::add(const string & uri, Response & res) {
  responseCache[uri] = res;
  if(contains(uri)){
    //move this uri to the front of LRU
    LRU_movetoFront(uri);
  }else if(capacity<=load){
    //new uri
    //delete one uri from LRU and add to the front 
    LRU_popEnd();
    LRU_addtoFront(uri);
    
  }else{
    //not full , add to the front
    load++;
    LRU_addtoFront(uri);
  }
  
}

bool Cache::contains(const string & uri){
  return responseCache.find(uri) != responseCache.end();
}

Response Cache::get(const string & uri){
  LRU_movetoFront(uri);
  return responseCache.at(uri);
}

void Cache::drop(const string &uri){
  responseCache.erase(uri);
  LRU_delete(uri);
}

void Cache::LRU_popEnd(){
  LRU.pop_back();
  --load;
};
void Cache::LRU_addtoFront(const string &uri){
  LRU.push_front(uri);
  ++load;
};
void Cache::LRU_delete(const string &uri){
  if(!LRU.empty()){
    std::list<string>::iterator it = LRU.begin();
    for(it;it!=LRU.end();++it){
      if(*it==uri){
        LRU.erase(it);
        load--;
        return;
      }
    }
  }
};
void Cache::LRU_movetoFront(const string &uri){
  LRU_delete(uri);
  LRU_addtoFront(uri);
};