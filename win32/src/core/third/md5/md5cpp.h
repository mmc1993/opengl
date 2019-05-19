#pragma once

extern "C" {
#include "md5.h"
}

#include <string>

struct MD5Ret {
    char str[16];
};

inline MD5Ret MD5(const void * data, const unsigned int len)
{
    MD5Ret ret;
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, data, len);
    MD5_Final((unsigned char *)ret.str, &ctx);
    return ret;
}

