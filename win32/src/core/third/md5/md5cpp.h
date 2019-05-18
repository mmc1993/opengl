#pragma once

extern "C" {
#include "md5.h"
}

#include <string>

union MD5Ret {
    struct {
        uint a;
        uint b;
        uint c;
        uint d;
    } xxx;
    char str[16];
};

inline MD5Ret MD5(const void * data, const uint len)
{
    MD5Ret ret;
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, data, len);
    MD5_Final((uchar *)ret.str, &ctx);
    return ret;
}

