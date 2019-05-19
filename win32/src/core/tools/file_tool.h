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

    inline uint GetFileLength(std::ofstream & ostream)
    {
        auto p = ostream.tellp();
        ostream.seekp(0, std::ios::end);
        auto r = ostream.tellp();
        ostream.seekp(p, std::ios::beg);
        return (uint)r;
    }
}