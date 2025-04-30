#include "../header/console_logger.h"
#include <fcntl.h>
#include <unistd.h>
#include <sys/file.h>
#include <sys/stat.h>
#include <sys/resource.h>

const std::string ConsoleLogger::RED = "\033[31m";
const std::string ConsoleLogger::GREEN = "\033[32m";
const std::string ConsoleLogger::RESET = "\033[0m";
std::mutex ConsoleLogger::thread_mutex;
std::ofstream ConsoleLogger::log_file;

void ConsoleLogger::init() {
    if (log_file.is_open()) {
        log_file.close();
    }
    mkdir("logs", 0755);

    std::stringstream filename;
    filename << "logs/process_" << getpid() << ".log";
    log_file.open(filename.str(), std::ios::out);
    
    if (!log_file.is_open()) {
        // Add error handling here
        // Check system file descriptor limits
        struct rlimit rlim;
        getrlimit(RLIMIT_NOFILE, &rlim);
        std::cerr << "Current fd limit: " << rlim.rlim_cur << "/" << rlim.rlim_max << std::endl;

        // Fallback to current directory
        filename.str("");
        filename << "process_" << getpid() << ".log";
        log_file.open(filename.str(), std::ios::out | std::ios::app);
    }
}

void ConsoleLogger::close() {
    if (log_file.is_open()) {
        log_file.close();
    }
}

std::string ConsoleLogger::get_current_timestamp() {
    // Get current time with millisecond precision
    auto now = std::chrono::system_clock::now();
    auto now_ms = std::chrono::duration_cast<std::chrono::milliseconds>(
        now.time_since_epoch()) % 1000;
    
    // Convert to time_t for the seconds part
    auto now_time_t = std::chrono::system_clock::to_time_t(now);
    auto now_tm = *std::localtime(&now_time_t);
    
    // Format the time including seconds
    std::stringstream ss;
    ss << std::put_time(&now_tm, "%Y-%m-%d %H:%M:%S");
    
    // Append milliseconds
    ss << '.' << std::setfill('0') << std::setw(3) << now_ms.count();
    
    return ss.str();
}

void ConsoleLogger::log(const std::string& message) {
    //std::lock_guard<std::mutex> lock_guard(thread_mutex);
    if (!log_file.is_open()) init();

    std::string timestamp = get_current_timestamp();
    log_file << "[" << timestamp << "] - " << "[PID:" << getpid() << "] " << "[THREAD ID:" << std::to_string(pthread_self()) << "] " << message << std::endl;
    // Also print to console
    std::cout << "[" << timestamp << "] - " << "[PID:" << getpid() << "] " << "[THREAD ID:" << std::to_string(pthread_self()) << "] " << message << std::endl;
}

void ConsoleLogger::print(const std::string& message) {
    //std::lock_guard<std::mutex> lock_guard(thread_mutex);
    if (!log_file.is_open()) init();

    std::string timestamp = get_current_timestamp();
    log_file << "[" << timestamp << "] - " << "[PID:" << getpid() << "] " << "[THREAD ID:" << std::to_string(pthread_self()) << "] " << message << std::endl;
    // Console gets colors
    std::cout << "[" << timestamp << "] - " << "[PID:" << getpid() << "] " << "[THREAD ID:" << std::to_string(pthread_self()) << "] " << GREEN << message << RESET << std::endl;
}

void ConsoleLogger::error(const std::string& message) {
    //std::lock_guard<std::mutex> lock_guard(thread_mutex);
    if (!log_file.is_open()) init();

    std::string timestamp = get_current_timestamp();
    log_file << "[" << timestamp << "] - " << "ERROR: " "[PID:" << getpid() << "] " << message << std::endl;
    // Console gets colors
    std::cerr << "[" << timestamp << "] - " << "[PID:" << getpid() << "] " << RED << "ERROR: " << message << RESET << std::endl;
}