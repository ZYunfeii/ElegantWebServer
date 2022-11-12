#include "redispool.h"

RedisPool* RedisPool::redisPool_ = nullptr;
std::mutex RedisPool::mtx_;

RedisPool::RedisPool() {}
RedisPool::~RedisPool() {}

RedisPool* RedisPool::instance() {
    if (redisPool_ == nullptr) {
        std::lock_guard<std::mutex> lk(mtx_);
        if (redisPool_ == nullptr) {
            redisPool_ = new RedisPool;
        }
    }
    return redisPool_;
}

void RedisPool::init(int connNum, char* host, int port) {
    for (int i = 0; i < connNum; ++i) {
        RedisCache* rc = new RedisCache;
        rc->init(host, port);
        connQueue_.push(rc);
    }
    maxConnNum_ = connNum;
    sem_init(&semId_, 0, maxConnNum_);
    LOG_INFO("Redis pool init completely. Connection number: %d", maxConnNum_);
}

RedisCache* RedisPool::getRedisCache() {
    RedisCache* rc = nullptr;
    if(connQueue_.empty()){
        LOG_WARN("Redis pool busy!");
        return nullptr;
    }
    sem_wait(&semId_); // P
    {
        std::lock_guard<std::mutex> lk(mtx_);
        rc = connQueue_.front();
        connQueue_.pop();
    }
    return rc;
}

void RedisPool::freeRedisCache(RedisCache* rc) {
    assert(rc);
    std::lock_guard<std::mutex> lk(mtx_);
    connQueue_.push(rc);
    sem_post(&semId_); // V
}