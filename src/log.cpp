#include <fstream>
#include <chrono>
#include <ctime>
#include <mutex>

std::ofstream logFile("/var/log/erss/proxy.log", std::ios::app); // create or open the log file in append mode
std::mutex logMutex; // declare a mutex for synchronizing access to the log file

void logMessage(const std::string & message) {
    // lock the mutex before accessing the log file
    std::lock_guard<std::mutex> lock(logMutex);

    // write the log message to the log file
    logFile << message << std::endl;
}