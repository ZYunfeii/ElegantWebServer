#ifndef REDISCACHE_H
#define REDISCACHE_H

#include <hiredis/hiredis.h>
#include <unordered_map>
#include <string>
#include <mutex>
#include "../log/log.h"

class RedisCache {
public:
    static RedisCache* Instance();
    bool init(const char* ip, int port);
    bool setKeyVal(std::string key, std::string val) const;
    std::string getKeyVal(std::string key) const;
    bool existKey(std::string key) const;
    bool incr(std::string key) const;
    bool flushDB() const;
    bool check() const;
    bool delKey(std::string key) const;
private:
    RedisCache();
    ~RedisCache();
    redisContext* ctx_;
    static RedisCache* cache_;
    static std::mutex mtx_;
    static std::mutex rmtx_;
};

#endif 