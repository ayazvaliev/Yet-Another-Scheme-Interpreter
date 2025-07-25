#pragma once

#include <string>

class Interpreter {
public:
    std::string Run(const std::string&);
    ~Interpreter();
private:
    std::string PreprocessInputStr(std::string str);
};
