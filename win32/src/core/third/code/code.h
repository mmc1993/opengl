#pragma once

#include <string>
#include "../../type.h"

class Code {
private:
    //  MD5
    typedef struct
    {
        uint count[2];
        uint state[4];
        uchar buffer[64];
    } MD5_CTX;

    static void MD5Beg(MD5_CTX *context);
    static void MD5End(MD5_CTX *context, uchar digest[16]);
    static void MD5Run(MD5_CTX *context, const uchar *input, const uint len);
    static void MD5Decode(uint  *output, const uchar *input, const uint len);
    static void MD5Encode(uchar *output, const uint  *input, const uint len);
    static void MD5Transform(uint state[4], const uchar block[64]);

    //  Base64
    static uint Base64Length(uint len);
    static void Base64Encode(const uchar * data, const uint len, uchar * output);

public:
    static std::string MD5Encode(const uchar * data, const uint len)
    {
        uchar md5[16] = { 0 };
        MD5_CTX  ctx;
        MD5Beg(&ctx);
        MD5Run(&ctx, data, len);
        MD5End(&ctx, md5);
        return std::string((const char *)md5, sizeof(md5));
    }

    static std::string Base64Encode(const uchar * data, const uint len)
    {
        auto outlen = Base64Length(len);
        std::string output(outlen,'\0');
        Base64Encode(data, len, (uchar *)output.data());
        return output;
    }
};
