#ifndef __COOKIE_H__
#define __COOKIE_H__

#include "md5.h"
#include <vector>
#include <map>
// 没用unordered_map 键为MD5码较长 会报错 且查询效率此时不如RB-tree based map
class Cookie{
public:
    static Cookie *get_instance(){
        static Cookie instance;
        return &instance;
    }
    void add_user_str(const std::string &user_str){
        user_cookies_map_.insert(std::pair<std::string, std::string>(MD5::md5_encryption(user_str), user_str));
    }
    std::string find_user_from_md5(std::string &md5) const{
        if(user_cookies_map_.find(md5) != user_cookies_map_.end())
            return user_cookies_map_.at(md5); // if not find md5, it will raise an error because of using ".at"
        else
            return "No user find"; // 正常情况不会进这 除非有人改浏览器cookie伪装访问服务器
    }
    void init_instance_from_vec(const std::vector<std::string> &user_str_vec){
        for(auto &user_str : user_str_vec){
            user_cookies_map_.insert(std::pair<std::string, std::string>(MD5::md5_encryption(user_str), user_str));
        }
    }
private:
    Cookie(){};
    std::map<std::string, std::string> user_cookies_map_;
};

#endif