#include "rediscache.h"
#include <iostream>

void test() {
    RedisCache* rc = new RedisCache();
    rc->init("127.0.0.1", 6379);
    bool res = rc->setKeyVal("yunfei", "22");
    if (!res) {
        std::cout << "fail!" << std::endl;
        return;
    }
    std::cout << "set success!" << std::endl;
    std::string str = rc->getKeyVal("yunfei");
    if (!res) {
        std::cout << "fail!" << std::endl;
        return;
    }
    std::cout << str << std::endl;

    res = rc->existKey("yunfei");

    res = rc->delKey("yunfei");
    if (!res) {
        std::cout << "fail del key!" << std::endl;
        return;
    }
    std::cout << "del key success!" << std::endl;

    rc->getKeyVal("y");

    // res = rc->flushDB();
    // if (!res) {
    //     std::cout << "flush fail!" << std::endl;
    //     return;
    // }
    // std::cout << "flush success!" << std::endl;
}

int main(int argc, char** argv) {
    // Log::Instance()->init(1, "./log", ".log", 1024);
    test();
    return 0;
}