#pragma once

using iint = GLint;
using uint = GLuint;

class NoCopy {
public:
    NoCopy() {}
    NoCopy(const NoCopy &) = delete;
    NoCopy & operator=(const NoCopy &) = delete;
};