#pragma once

#include "File.hpp"

class Compare
{
public:
    std::string bak_path_;

    Compare(std::string bak_path);
    ~Compare();

    bool compare();
};