#ifndef MVK_LOGGER_H_
#define MVK_LOGGER_H_

#include <iostream>
#include <fstream>
#include <string>
#include <vector>

class Logger {
public:
    explicit Logger(std::size_t n = 1024);

    Logger(const Logger&) = delete;
    Logger(Logger&&) = delete;
    Logger& operator=(const Logger&) = delete;
    Logger& operator=(Logger&&) = delete;

    void log(const std::string& str);

    void write(const char* filename = "log.txt") const;

private:
    std::vector<std::string> arr;
};

extern Logger mvk_log;
void mLog(const std::string&);

#endif // MVK_LOGGER_H_
