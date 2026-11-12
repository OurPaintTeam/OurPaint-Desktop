#include "Logger.h"

Logger::Logger(std::size_t n)
{
    arr.reserve(n);
}

void Logger::log(const std::string& str)
{
    arr.push_back(str);
    std::cout << str << '\n';
}

void Logger::write(const char* filename) const
{
    std::ofstream output(filename);
    if (!output) {
        throw std::runtime_error("failed to open log file");
    }
    for (auto& s : arr) {
        output << s << '\n';
    }
}

Logger mvk_log;

void mLog(const std::string& str) {
    mvk_log.log(str);
}
