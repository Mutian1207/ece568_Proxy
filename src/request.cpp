#include"request.hpp"
#include <string>
namespace http = boost::beast::http;
#include <iomanip>
using sysclock_t = std::chrono::system_clock;
Request::Request(const char *ori_req): request(ori_req){
    //parse the string format request using boost.beast
    http::request_parser<http::string_body> parser;
    //parserd request 
    http::request<http::string_body> parsed_request;
    // error_code for error checking 
    boost::beast::error_code ec;
    //call the parser.put() method to parse the request from the string, 
    //passing in a buffer pointing to the string data and the length of the string.
    parser.put(boost::asio::buffer(request.data(), request.size()),ec);
    if(ec){
        // ??????? deal with this.
        std::cerr<<"Error parsing HTTP request"<<ec.message()<<std::endl;
        return;
    }
    //call parser.get() to retrieve the parsed request from the parser
    parsed_request=parser.get();
    
    //method = parsed_request.method_string();
    method = (std::string) parsed_request.method_string();
    url_target = (std::string) parsed_request.target();
    //version = parsed_request.version();
    //including the hostname and port
    host = (std::string) parsed_request[http::field::host];
    size_t port_start = host.find(":");
    if(port_start!=std::string::npos){
        hostname = host.substr(0,port_start);
        port = host.substr(port_start+1);
    }else{
        hostname = host;
        //port default 80 for http 443 for https
        //port = "80";
        port = "";
    }
    user_agent=(std::string) parsed_request[http::field::user_agent];
    body = parsed_request.body();
    int firstline_end = request.find("\r\n");
    firstline = request.substr(0,firstline_end);
    std::time_t now = sysclock_t::to_time_t(sysclock_t::now());
    struct tm*timeinfo;
    time(&now);
    timeinfo = localtime(&now);
    sent_time = asctime(timeinfo);
    sent_time.erase(sent_time.length() - 1);  // delete "\n"

    //std::cout<<"method: "<<method<<std::endl;
    //std::cout<<"target: "<<url_target<<std::endl;
    //std::cout<<"version: "<<version<<std::endl;
    //std::cout<<"host: "<<host<<std::endl;
    //std::cout<<"hostname: "<<hostname<<std::endl;
    //std::cout<<"port:"<<port<<std::endl;
    //std::cout<<"user_agent: "<<user_agent<<std::endl;
    //std::cout<<"header: "<<header<<std::endl;
    //std::cout<<"body:"<<body<<std::endl;


};