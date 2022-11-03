#include "rediscache.h"

RedisCache* RedisCache::cache_ = nullptr;
std::mutex RedisCache::mtx_;

RedisCache::RedisCache() : ctx_(nullptr) {}

RedisCache* RedisCache::Instance() {
    if (cache_ == nullptr) {
        std::lock_guard<std::mutex> lk(mtx_);
        if (cache_ == nullptr) {
            cache_ = new RedisCache;
        }
    }
    return cache_;
}

RedisCache::~RedisCache () {
    delete cache_;
}

bool RedisCache::init(const char* ip, int port) {
    ctx_ = redisConnect(ip, port); // redis 默认6379端口
    if (ctx_->err) {
        redisFree(ctx_);
        LOG_ERROR("Connect to redisServer fail");
        return false;
    }
    LOG_INFO("Connect to redisServer Success");
    return true;
}

bool RedisCache::setKeyVal(std::string key, std::string val) const {
    if (!check()) {
        LOG_ERROR("No connection to Redis.");
    }
    std::string command = "set ";
    command += key;
    command += " ";
    command += val;
    redisReply* r = (redisReply*)redisCommand(ctx_, command.c_str()); 
    if (r == nullptr) {
        LOG_ERROR("Excute command %s failure.", command.c_str());
        return false;
    }
    LOG_INFO("Excute command %s successfully.", command.c_str());
    return true;
}

std::string RedisCache::getKeyVal(std::string key) const {
    if (!check()) {
        LOG_ERROR("No connection to Redis.");
    }
    std::string command = "get ";
    command += key;
    redisReply* r = (redisReply*)redisCommand(ctx_, command.c_str()); 
    if (r->type == REDIS_REPLY_NIL) {
        freeReplyObject(r);
        return "nil";
    }
    if (r->type != REDIS_REPLY_STRING) {
        LOG_ERROR("Failed to execute command %s.", command.c_str());
        freeReplyObject(r); 
        redisFree(ctx_); 
        return ""; 
    }
    std::string res(r->str, r->str + r->len);
    freeReplyObject(r);
    return res;
}

bool RedisCache::existKey(std::string key) const {
    if (!check()) {
        LOG_ERROR("No connection to Redis.");
    }
    std::string command = "exists ";
    command += key;
    redisReply* r = (redisReply*)redisCommand(ctx_, command.c_str()); 
    if (r->type != REDIS_REPLY_INTEGER) {
        LOG_ERROR("Failed to execute command %s.", command.c_str());
        freeReplyObject(r); 
        redisFree(ctx_); 
        return false; 
    }
    freeReplyObject(r);
    return true;
}

bool RedisCache::flushDB() const {
    if (!check()) {
        LOG_ERROR("No connection to Redis.");
    }
    std::string command = "flushdb";
    redisReply* r = (redisReply*)redisCommand(ctx_, command.c_str()); 
    if (r->type != REDIS_REPLY_STATUS) {
        LOG_ERROR("Failed to execute command %s.", command.c_str());
        freeReplyObject(r); 
        redisFree(ctx_); 
        return false; 
    }
    freeReplyObject(r);
    return true;
}

bool RedisCache::delKey(std::string key) const {
    if (!check()) {
        LOG_ERROR("No connection to Redis.");
    }
    std::string command = "del ";
    command += key;
    redisReply* r = (redisReply*)redisCommand(ctx_, command.c_str());
    if (r->type != REDIS_REPLY_INTEGER) {
        LOG_ERROR("Failed to execute command %s.", command.c_str());
        freeReplyObject(r); 
        redisFree(ctx_); 
        return false; 
    }
    freeReplyObject(r);
    return true;
}

bool RedisCache::check() const {
    if (ctx_ == nullptr || ctx_->err) {
        return false;
    }
    return true;
}

