#ifndef __MD5_H__
#define __MD5_H__

#include <string.h>
#include <iostream>
#include <openssl/md5.h>

class MD5{
public:
    static std::string md5_encryption(const std::string &src){
        MD5_CTX ctx;
        std::string md5_string;
        unsigned char md[16] = { 0 };
        char tmp[33] = { 0 };
    
        MD5_Init( &ctx );
        MD5_Update( &ctx, src.c_str(), src.size() );
        MD5_Final( md, &ctx );
    
        for( int i = 0; i < 16; ++i )
        {   
            memset(tmp, 0x00, sizeof(tmp));
            sprintf(tmp, "%02X", md[i]);
            md5_string += tmp;
        }   
        return md5_string;
    }
private:
    MD5();
};

#endif