#ifndef REDISPOOL_H
#define REDISPOOL_H

#include <mutex>
#include <queue>
#include <assert.h>
#include <semaphore.h>
#include "rediscache.h"

class RedisPool {
public:
    static RedisPool* instance();
    void init(int connNum, char* host, int port);
    RedisCache* getRedisCache();
    void freeRedisCache(RedisCache* rc);
private:
    RedisPool();
    ~RedisPool();
    static RedisPool* redisPool_;
    static std::mutex mtx_;
    sem_t semId_;
    int maxConnNum_;
    std::queue<RedisCache*> connQueue_;
};

#endif