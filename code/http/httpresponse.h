/*
 * @Author       : mark
 * @Date         : 2020-06-25
 * @copyleft Apache 2.0
 */ 
#ifndef HTTP_RESPONSE_H
#define HTTP_RESPONSE_H

#include <unordered_map>
#include <fcntl.h>       // open
#include <unistd.h>      // close
#include <sys/stat.h>    // stat
#include <sys/mman.h>    // mmap, munmap

#include "../buffer/buffer.h"
#include "../log/log.h"
#include "httprequest.h"

class HttpResponse {
public:
    HttpResponse();
    ~HttpResponse();

    void Init(const std::string& srcDir, std::string& path, std::shared_ptr<cookie> cke, bool isKeepAlive = false, int code = -1);
    void MakeResponse(Buffer& buff);
    void UnmapFile();
    char* File();
    size_t FileLen() const;
    void ErrorContent(Buffer& buff, std::string message);
    int Code() const { return code_; }
    bool ifTransNotFile();
    char* GetTransStrToCharPtr();
    size_t StrLen() const;

private:
    void AddStateLine_(Buffer &buff);
    void AddHeader_(Buffer &buff);
    void AddContent_(Buffer &buff);

    void ErrorHtml_();
    std::string GetFileType_();

    int code_;
    bool isKeepAlive_;

    std::string path_;
    std::string srcDir_;
    
    char* mmFile_; 
    struct stat mmFileStat_;

    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
    static const std::unordered_set<std::string> STRING_GET;

    std::shared_ptr<cookie> mCookie_;
    bool ifTransNotFile_;
    std::string strTrans_;
};


#endif //HTTP_RESPONSE_H