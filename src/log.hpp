#pragma once

#include <iomanip>
#include <iostream>
#include <sstream>

#ifndef NDEBUG

#define INSERTER(msgs) [&](std::ostream& os) { os << msgs; }

#define debug(msgs) log("DEBUG", __FILE__, __LINE__, INSERTER(msgs));
#define info(msgs) log("INFO", __FILE__, __LINE__, INSERTER(msgs));

#else

#define debug(inserter) nop()
#define info(inserter) nop()

#endif

namespace Log
{

std::string file_and_line(std::string, int);

void log(
        const std::string level,
        const std::string file,
        const int line,
        const std::function<void(std::ostream&)> inserter)
{
    std::cout
        << std::left
        << std::setw(6) << level
        << std::setw(32) << file_and_line(file, line);
    inserter(std::cout);
    std::cout << std::endl;
}

std::string file_and_line(std::string file, const int line)
{
    file.erase(0, 4); // Remove "src/"
    std::ostringstream oss;
    oss << file << ':' << line;
    return oss.str();
}

void nop() {}

}
