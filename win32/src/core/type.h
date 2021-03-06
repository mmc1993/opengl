#pragma once

using iint = int;
using uint = unsigned int;
using uint8 = unsigned char;

using ichar = char;
using uchar = unsigned char;

class NonCopy {
public:
    NonCopy() {}
    NonCopy(const NonCopy &) = delete;
    NonCopy & operator=(const NonCopy &) = delete;
};