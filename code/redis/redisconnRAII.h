#ifndef REDISCONNRII_H
#define REDISCONNRII_H

#include "redispool.h"

class RedisConnRAII {
public:
    RedisConnRAII(RedisCache** rc, RedisPool* rp) {
        assert(rp);
        *rc = rp->getRedisCache();
        rc_ = *rc;
        rp_ = rp;
    }
    ~RedisConnRAII() {
        if (rc_) {
            rp_->freeRedisCache(rc_);
        }
    }
private:
    RedisCache* rc_;
    RedisPool* rp_;
};

#endif