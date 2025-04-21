#ifndef CONSOLE_LOGGER_H
#define CONSOLE_LOGGER_H

#include <iostream>
#include <fstream>
#include <string>
#include <mutex>
#include <unistd.h>
#include <sstream>
#include <chrono>
#include <iomanip>


class ConsoleLogger {
public:
    static const std::string RED;
    static const std::string GREEN;
    static const std::string RESET;
    static std::mutex thread_mutex;
    static std::ofstream log_file;

    static void init();
    static void close();
    static std::string get_current_timestamp();
    static void log(const std::string& message);
    static void print(const std::string& message);
    static void error(const std::string& message);
};

#endif // CONSOLE_LOGGER_H