#pragma once

using iint = int;
using uint = unsigned int;

using ichar = char;
using uchar = unsigned char;

class NonCopy {
public:
    NonCopy() {}
    NonCopy(const NonCopy &) = delete;
    NonCopy & operator=(const NonCopy &) = delete;
};