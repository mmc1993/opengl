#pragma once

using iint = GLint;
using uint = GLuint;

using ichar = char;
using uchar = unsigned char;

class NoCopy {
public:
    NoCopy() {}
    NoCopy(const NoCopy &) = delete;
    NoCopy & operator=(const NoCopy &) = delete;
};