#pragma once

template <class T>
size_t GL_UBO_SIZEOF_STD140

#define GL_UBO_SIZEOF_STD140(var)               (sizeof(var) > 8? 16: sizeof(var) > 4? 8: 4)
#define GL_UBO_OFFSETOF_STD140(offset, size)     (size + 1 + offset) / size * size


