#pragma once

#include "../include.h"

namespace file_tool {
    inline uint GetFileLength(std::ifstream & istream)
    {
        auto p = istream.tellg();
        istream.seekg(0, std::ios::end);
        auto r = istream.tellg();
        istream.seekg(p, std::ios::beg);
        return (uint)r;
    }
}