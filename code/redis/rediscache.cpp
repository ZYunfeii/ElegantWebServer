#include "rediscache.h"

/* hiredis.h中的context不是线程安全的，并发会有问题 */
std::mutex RedisCache::mtx_;

RedisCache::RedisCache() : ctx_(nullptr) {}

RedisCache::~RedisCache () {
}

bool RedisCache::init(const char* host, int port) {
    ctx_ = redisConnect(host, port); // redis 默认6379端口
    if (ctx_->err) {
        redisFree(ctx_);
        LOG_ERROR("Connect to redisServer fail");
        return false;
    }
    LOG_INFO("Connect to redisServer Success");
    return true;
}

bool RedisCache::setKeyVal(std::string key, std::string val) const {
    std::lock_guard<std::mutex> lk(mtx_);
    redisReply* r = (redisReply*)redisCommand(ctx_, "set %s %b", key.c_str(), val.c_str(), val.length()); 
    if (r == nullptr) {
        LOG_ERROR("Excute set %s failure.", key.c_str());
        freeReplyObject(r);
        return false;
    }
    freeReplyObject(r);
    return true;
}

bool RedisCache::setKeyVal(std::string key, char* val, int sz) const {
    std::lock_guard<std::mutex> lk(mtx_);
    redisReply* r = (redisReply*)redisCommand(ctx_, "set %s %b", key.c_str(), val, sz); 
    if (r == nullptr) {
        LOG_ERROR("Excute set %s failure.", key.c_str());
        freeReplyObject(r);
        return false;
    }
    freeReplyObject(r);
    return true;
}

std::string RedisCache::getKeyVal(std::string key) const {
    std::lock_guard<std::mutex> lk(mtx_);
    redisReply* r = (redisReply*)redisCommand(ctx_, "get %s", &key[0]); // .c_str() bug?
    if (r->type == REDIS_REPLY_NIL) {
        LOG_INFO("Key %s is nil.", key.data());
        freeReplyObject(r);
        return noStr;
    }
    if (r->type != REDIS_REPLY_STRING) {
        LOG_ERROR("Failed to get key %s.", key.data());
        freeReplyObject(r); 
        return ""; 
    }
    std::string res(r->str, r->str + r->len);
    freeReplyObject(r);
    return res;
}

bool RedisCache::existKey(std::string key) const {
    std::lock_guard<std::mutex> lk(mtx_);
    redisReply* r = (redisReply*)redisCommand(ctx_, "exists %s", key.data());
    if (r->type != REDIS_REPLY_INTEGER) {
        LOG_ERROR("Failed to execute exists %s.", key.c_str());
        freeReplyObject(r); 
        return false; 
    }
    int res = r->integer;
    freeReplyObject(r);
    return res == 1; 
}

bool RedisCache::incr(std::string key) const {
    std::lock_guard<std::mutex> lk(mtx_);
    std::string command = "incr ";
    command += key;
    redisReply* r = (redisReply*)redisCommand(ctx_, command.data()); 
    if (r->type != REDIS_REPLY_INTEGER) {
        LOG_ERROR("Failed to execute command %s.", command.c_str());
        freeReplyObject(r); 
        return false; 
    }
    freeReplyObject(r);
    return true;
}

bool RedisCache::flushDB() const {
    std::lock_guard<std::mutex> lk(mtx_);
    std::string command = "flushdb";
    redisReply* r = (redisReply*)redisCommand(ctx_, command.data()); 
    if (r->type != REDIS_REPLY_STATUS) {
        LOG_ERROR("Failed to execute command %s.", command.c_str());
        freeReplyObject(r); 
        return false; 
    }
    freeReplyObject(r);
    return true;
}

bool RedisCache::delKey(std::string key) const {
    std::lock_guard<std::mutex> lk(mtx_);
    std::string command = "del ";
    command += key;
    redisReply* r = (redisReply*)redisCommand(ctx_, command.c_str());
    if (r->type != REDIS_REPLY_INTEGER) {
        LOG_ERROR("Failed to execute command %s.", command.c_str());
        freeReplyObject(r); 
        return false; 
    }
    freeReplyObject(r);
    return true;
}

bool RedisCache::listPush(std::string key, std::string val) const {
    std::lock_guard<std::mutex> lk(mtx_);
    redisReply* r = (redisReply*)redisCommand(ctx_, "lpush %s %s", key.c_str(), val.c_str(), val.length()); 
    if (r->type == REDIS_REPLY_ERROR) {
        LOG_ERROR("Excute list push %s failure.", key.c_str());
        freeReplyObject(r);
        return false;
    }
    freeReplyObject(r);
    return true;
}

std::vector<std::string> RedisCache::listRange(std::string key, int left, int right) const {
    std::lock_guard<std::mutex> lk(mtx_);
    redisReply* r = (redisReply*)redisCommand(ctx_, "lrange %s %d %d", key.c_str(), left, right); 
    if (r->type == REDIS_REPLY_ERROR) {
        LOG_ERROR("Excute list push %s failure.", key.c_str());
        freeReplyObject(r);
        return {};
    }
    std::vector<std::string> res;
    for (int i = 0; i < r->elements; ++i) {
        res.push_back(std::move(r->element[i]->str));
    }
    freeReplyObject(r);
    return res;
}


