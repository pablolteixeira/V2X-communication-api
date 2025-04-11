#ifndef CONSOLE_LOGGER_H
#define CONSOLE_LOGGER_H

#include <iostream>
#include <string>

class ConsoleLogger
{
public:
    static const std::string RED;
    static const std::string GREEN;
    static const std::string RESET;

    static void print(const std::string& message);

    static void error(const std::string& message);
};

#endif // CONSOLE_LOGGER_H