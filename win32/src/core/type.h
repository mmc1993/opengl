#pragma once

using iint = GLint;
using uint = GLuint;

using ichar = char;
using uchar = unsigned char;

class NonCopy {
public:
    NonCopy() {}
    NonCopy(const NonCopy &) = delete;
    NonCopy & operator=(const NonCopy &) = delete;
};