#include "../header/console_logger.h"

const std::string ConsoleLogger::RED = "\033[31m";
const std::string ConsoleLogger::GREEN = "\033[32m";
const std::string ConsoleLogger::RESET = "\033[0m";

void ConsoleLogger::print(const std::string& message) {
    std::cout << GREEN << message << RESET << std::endl;
}

void ConsoleLogger::error(const std::string& message) {
    std::cerr << RED << "ERROR: " << message << RESET << std::endl;
}