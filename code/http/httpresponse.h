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
#include "../redis/redisconnRAII.h"

const std::string kComment = "comments";
const std::string kNumVisits = "numVisits";
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

private:
    void AddStateLine_(Buffer &buff);
    void AddHeader_(Buffer &buff);
    void AddContent_(Buffer &buff);
    inline void AddContentNotFile_(Buffer &buffer, RedisCache* rc);

    void ErrorHtml_();
    std::string GetFileType_();
    std::vector<std::string> ProcessPath_() const;

    void CheckRedis_();
    void MakeRedisResponse_(Buffer& buff);

    int code_;
    bool isKeepAlive_;

    std::string path_;
    std::string srcDir_;
    
    char* mmFile_; 
    struct stat mmFileStat_;

    std::shared_ptr<cookie> mCookie_;
    bool ifTransNotFile_;

    std::string redisFile_;
    std::string getStr_;
    std::string getSrc_;

public:
    static const std::unordered_map<std::string, std::string> SUFFIX_TYPE;
    static const std::unordered_set<std::string> NO_REDIS_CACHE;
    static const std::unordered_map<int, std::string> CODE_STATUS;
    static const std::unordered_map<int, std::string> CODE_PATH;
    static const std::unordered_map<std::string, int> STRING_GET;
};


#endif //HTTP_RESPONSE_H