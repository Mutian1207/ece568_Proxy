#include "response.hpp"

#include <boost/asio.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>

#include <chrono>
#include <cstring>
#include <ctime>
#include <iomanip>
#include <string>

using sysclock_t = std::chrono::system_clock;
// max-age max-stale mustrevalid nocache nostore private public
namespace http = boost::beast::http;
void Response::parse_response() {
  http::response_parser<http::string_body> response_parser;
  http::response<http::string_body> resp;
  boost::beast::error_code ec;

  response_parser.put(boost::asio::buffer(response, response.size()), ec);
  if (ec) {
    std::cout << ec.message() << std::endl;
  }
  // get parsed response
  resp = response_parser.get();
  // get header
  http::header<false, http::fields> & header = resp.base();
  std::cout << "response header:" << std::endl << header << std::endl;
  // get date
  auto header_date = header.find("Date");
  if (header_date != header.end()) {
    date = (std::string)header_date->value();
    std::cout << "Date found: " << date << std::endl;
  }
  else {
    std::time_t now = sysclock_t::to_time_t(sysclock_t::now());
    std::stringstream stream;
    stream << now;
    date = stream.str();
  }
  // get last-modified
  auto header_lastmd = header.find("Last-Modified");
  if (header_lastmd != header.end()) {
    last_modified = (std::string)header_lastmd->value();
    std::cout << "Last-Modified found: " << last_modified << std::endl;
  }
  else {
    std::time_t now = sysclock_t::to_time_t(sysclock_t::now());
    std::stringstream stream;
    stream << now;
    last_modified = stream.str();
  }
  
  // get first line status code
  int status_end = response.find("\r\n");
  status_code = response.substr(0, status_end);
  std::cout << "status code:" << status_code << std::endl;
  // get cache control

  auto header_cachecontrol = header.find("Cache-Control");
  if (header_cachecontrol != header.end()) {
    std::cout << "cache control found" << std::endl;
    std::string content = (std::string)header_cachecontrol->value();
    if (content.find("private") != std::string::npos) {
      is_private = 1;
      return;
    }
    else {
      is_private = 0;
    }
    if (content.find("no-store") != std::string::npos) {
      nostore = 1;
      return;
    }
    else {
      nostore = 0;
    }
    if (content.find("must-revalidate") != std::string::npos) {
      must_revalidation = 1;
    }
    else {
      must_revalidation = 0;
    }
    if (content.find("no-cache") != std::string::npos) {
      nocache = 1;
    }
    else {
      nocache = 0;
    }
    if (content.find("maxage") != std::string::npos) {
      // std::cout << "????" <<std::endl;
      // std::cout << content << std::endl;
      // std::cout << content.find("\n") << std::endl;
      // std::cout << content.find("s-maxage") <<std::endl;
      // std::cout << content.length()<<std::endl;
      int maxage_start = content.find("maxage") + sizeof("maxage=");
      int pos = maxage_start;
      for (; content[pos] != ',' && pos < content.length(); pos++) {
        continue;
      }
      maxage = std::stoi(content.substr(maxage_start, pos));
    }
    else {
      maxage = INT_MAX;
    }
    if (content.find("max-stale") != std::string::npos) {
      int maxstale_start = content.find("max-stale") + sizeof("max-stale=");
      int pos = maxstale_start;
      for (; content[pos] != ',' && pos < content.length(); pos++) {
        continue;
      }
      maxstale = std::stoi(content.substr(maxstale_start, pos));
    }
    else {
      maxstale = INT_MAX;
    }
  }
  if(maxstale==INT_MAX){
    
  }
  time_t maxtime = parse_time(date) + +maxage+maxstale;
  struct tm * timeinfo;
  timeinfo = localtime(&maxtime);
  expire_time = asctime(timeinfo);
  expire_time.erase(expire_time.size()-1);
};

time_t Response::parse_time(std::string & origin_time) {
  struct tm tm;
  memset(&tm,0,sizeof(tm));
  std::istringstream iss(origin_time);
  iss >> std::get_time(&tm, "%a, %d %b %Y %H:%M:%S");
  time_t time = mktime(&tm);
  return time;
}
