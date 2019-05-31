#pragma once

#include "../type.h"

namespace number_tool {
    inline uchar * Conver16(const uchar * src, const uint len, uchar * out)
    {
        static auto TABLE = "0123456789abcdef";
        for (auto i = 0; i != len; ++i)
        {
            out[i * 2    ] = TABLE[src[i] >>  4];
            out[i * 2 + 1] = TABLE[src[i] & 0xF];
        }
        return out;
    }
}