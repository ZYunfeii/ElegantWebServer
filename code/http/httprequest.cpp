/*
 * @Author       : mark
 * @Date         : 2020-06-26
 * @copyleft Apache 2.0
 */ 
#include "httprequest.h"
#include "cookie.h"
using namespace std;

Cookie *m_cookie = Cookie::get_instance();        // 获取Cookie单例

const unordered_set<string> HttpRequest::DEFAULT_HTML{
            "/index", "/register", "/login",
             "/welcome", "/video", "/picture",
             "/blogindex", "/fileupload", "/filedownload",
             "/msgboard"};

const unordered_map<string, int> HttpRequest::DEFAULT_HTML_TAG {
            {"/register.html", 0}, {"/login.html", 1},  };

void HttpRequest::Init() {
    method_ = path_ = version_ = body_ = "";
    state_ = REQUEST_LINE;
    header_.clear();
    post_.clear();
    m_cookie_ = std::make_shared<cookie>();
    m_cookie_->isCookie_ = false;
    m_cookie_->sendCookieStr_ = "";
}

bool HttpRequest::IsKeepAlive() const {
    if(header_.count("Connection") == 1) {
        return header_.find("Connection")->second == "keep-alive" && version_ == "1.1";
    }
    return false;
}

bool HttpRequest::parse(Buffer& buff) {
    const char CRLF[] = "\r\n";
    if(buff.ReadableBytes() <= 0) {
        return false;
    }
    while(buff.ReadableBytes() && state_ != FINISH) { // 主状态机最终state = FINISH 退出循环
        const char* lineEnd = search(buff.Peek(), buff.BeginWriteConst(), CRLF, CRLF + 2); // 在buff中找到\r\n序列中\r的头指针
        if(state_ == BODY && header_["Content-Type"].find("multipart/form-data") != std::string::npos) { 
            lineEnd = buff.BeginWriteConst(); // 如果下一个状态是读消息体且为POST上传文件，则lineEnd直接取结尾
        } 
        std::string line(buff.Peek(), lineEnd);
        switch(state_) // 状态机
        {
            case REQUEST_LINE:
                if(!ParseRequestLine_(line)) { 
                    return false;
                }
                ParsePath_();
                break;    
            case HEADERS:
                ParseHeader_(line);
                if(buff.ReadableBytes() <= 2) { // 剩下\r\n没有数据部分就直接FINISH
                    state_ = FINISH;
                }
                break;
            case BODY:
                ParseBody_(line);
                break;
            default:
                break;
        }
        if(lineEnd == buff.BeginWrite()) { break; }
        buff.RetrieveUntil(lineEnd + 2); // 这里对buff已读部分进行跳过
    }   

    LOG_DEBUG("[%s], [%s], [%s]", method_.c_str(), path_.c_str(), version_.c_str());
    return true;
}

void HttpRequest::ParsePath_() {
    if(path_ == "/") {
        path_ = "/index.html"; 
    }else {
        for(auto &item: DEFAULT_HTML) { // html中href可能没加.html，因此在这加上
            if(item == path_) {
                path_ += ".html";
                break;
            }
        }
    }

}

bool HttpRequest::ParseRequestLine_(const string& line) {
    regex patten("^([^ ]*) ([^ ]*) HTTP/([^ ]*)$");
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {   
        method_ = subMatch[1];
        path_ = subMatch[2];
        version_ = subMatch[3];
        state_ = HEADERS; // 从状态机器转移状态
        return true;
    }
    LOG_ERROR("RequestLine Error");
    return false;
}

void HttpRequest::ParseHeader_(const string& line) {
    regex patten("^([^:]*): ?(.*)$");
    smatch subMatch;
    if(regex_match(line, subMatch, patten)) {
        header_[subMatch[1]] = subMatch[2];
    }
    else { // 不满足正则说明读到头和数据部分中间的空行 则下一行为数据
        state_ = BODY;
    }
}

void HttpRequest::ParseBody_(const string& line) {
    body_ = line;
    ParsePost_();
    state_ = FINISH;
    LOG_DEBUG("Body:%s, len:%d", line.c_str(), line.size());
}

int HttpRequest::ConverHex(char ch) {
    if(ch >= 'A' && ch <= 'F') return ch -'A' + 10;
    if(ch >= 'a' && ch <= 'f') return ch -'a' + 10;
    return ch;
}

void HttpRequest::ParsePost_() { // 只有POST有数据部分(BODY)
    LOG_DEBUG("ParaPost");
    if(method_ == "POST" && header_["Content-Type"] == "application/x-www-form-urlencoded") {
        ParseFromUrlencoded_(); // 获取POST键值对
        if(DEFAULT_HTML_TAG.count(path_)) {
            int tag = DEFAULT_HTML_TAG.find(path_)->second;
            LOG_DEBUG("Tag:%d", tag);
            if(tag == 0 || tag == 1) {
                bool isLogin = (tag == 1);
                if(UserVerify(post_["username"], post_["password"], isLogin)) {
                    path_ = "/welcome.html";
                } 
                else {
                    path_ = "/error.html";
                }
            }
        }
    }else if(method_ == "POST" && header_["Content-Type"].find("multipart/form-data") != std::string::npos) { // todo
        ParseFileUpLoadBody_();
    }   
}

/*
multipart/form-data; boundary=----WebKitFormBoundaryxOLnWy9YBRuoNy4d
------WebKitFormBoundaryxOLnWy9YBRuoNy4d
Content-Disposition: form-data; name="filename"; filename="test.txt"
Content-Type: text/plain

hello world
------WebKitFormBoundaryxOLnWy9YBRuoNy4d--
*/
void HttpRequest::ParseFileUpLoadBody_() {
    std::string find_str = "boundary=";
    std::size_t idx = header_["Content-Type"].find(find_str);
    if (idx == std::string::npos) return; // 没找到boundary信息 报文有误
    std::string boundary(header_["Content-Type"].begin() + idx + find_str.size(), header_["Content-Type"].end());
    int boundary_len = boundary.size();
    find_str = "Content-Type:";
    idx = body_.find(find_str);
    while (!(body_[idx] == '\r' && body_[idx + 1] == '\n')) ++idx;
    idx += 4; // 两对\r\n
    std::string file_data(body_.begin() + idx, body_.end() - boundary_len - 6); // 结尾 \r\n + “--” + 开头"--"
    std::string file_name = "./user-msgs/default";
    if (header_.find("Cookie") != header_.end()) { // 存在cookie
        std::string cke = header_["Cookie"];
        std::string sessionid = "";
        size_t idx = cke.find("=");
        if(idx != std::string::npos) {
            sessionid = cke.substr(idx + 1, cke.size() - idx - 1);
        }
        std::string user_find = m_cookie->find_user_from_md5(sessionid);
        if (!sessionid.empty() && user_find != "No user find") {
            file_name = "./user-msgs/" + user_find;
        }
    }
    const char *file_begin_write = file_data.data();
    FILE *file;
    file = fopen(file_name.c_str(), "w");
    fwrite(file_begin_write, file_data.size(), 1, file);
    fflush(file);
    fclose(file);
}

void HttpRequest::ParseFromUrlencoded_() {
    if(body_.size() == 0) { return; }

    string key, value;
    int num = 0;
    int n = body_.size();
    int i = 0, j = 0;

    for(; i < n; i++) { // POST请求数据部分是编码了的，下面是一些常见的解码操作
        char ch = body_[i];
        switch (ch) {
        case '=': // key1=value1&key2=value2
            key = body_.substr(j, i - j);
            j = i + 1;
            break;
        case '+': // 上传文本表单空格用的'+'
            body_[i] = ' ';
            break;
        case '%': // 暂时不知道这个符号
            num = ConverHex(body_[i + 1]) * 16 + ConverHex(body_[i + 2]);
            body_[i + 2] = num % 10 + '0';
            body_[i + 1] = num / 10 + '0';
            i += 2;
            break;
        case '&': // key1=value1&key2=value2
            value = body_.substr(j, i - j);
            j = i + 1;
            post_[key] = value;
            LOG_DEBUG("%s = %s", key.c_str(), value.c_str());
            break;
        default:
            break;
        }
    }
    assert(j <= i);
    if(post_.count(key) == 0 && j < i) { // 最后一个键值对不是&结尾 特殊处理一下
        value = body_.substr(j, i - j);
        post_[key] = value;
    }
}

bool HttpRequest::UserVerify(const string &name, const string &pwd, bool isLogin) {
    if(name == "" || pwd == "") { return false; }
    LOG_INFO("Verify name:%s pwd:%s", name.c_str(), pwd.c_str());
    RedisCache* rc = nullptr;
    RedisConnRAII(&rc, RedisPool::instance());
    assert(rc);
    bool flag = false;
    if(!isLogin) { flag = true; }
    if (rc->existKey(name)) {
        string password = rc->getKeyVal(name);
        LOG_DEBUG("Redis get user pwd: %s", password.data());
        if(isLogin) { // 登录
            if(pwd == password) {  // 登录，密码正确
                flag = true;
                m_cookie_->isCookie_ = true; 
                m_cookie_->sendCookieStr_ = MD5::md5_encryption(name);
                m_cookie->add_user_str(name);
            }
            else { // 登录，但密码错误
                flag = false;
                LOG_DEBUG("pwd error!");
            }
        }  // 注册，如果Redis中有对应密码，则错误
        else { 
            flag = false; 
            LOG_DEBUG("user used!");
        }
    }
    /* 注册行为 且 用户名未被使用*/
    if(!isLogin && flag == true) {
        LOG_DEBUG("Regirster!"); 
        if(rc->setKeyVal(name, pwd)) { 
            LOG_DEBUG( "Insert redis error!");
            flag = false; 
        }
        flag = true;
        m_cookie_->isCookie_ = true; 
        m_cookie_->sendCookieStr_ = MD5::md5_encryption(name);;
        m_cookie->add_user_str(name);
    }
    LOG_DEBUG( "UserVerify success!!");
    return flag;
}

std::string HttpRequest::path() const{
    return path_;
}

std::string& HttpRequest::path(){
    return path_;
}
std::string HttpRequest::method() const {
    return method_;
}

std::string HttpRequest::version() const {
    return version_;
}

std::string HttpRequest::GetPost(const std::string& key) const {
    assert(key != "");
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

std::string HttpRequest::GetPost(const char* key) const {
    assert(key != nullptr);
    if(post_.count(key) == 1) {
        return post_.find(key)->second;
    }
    return "";
}

std::shared_ptr<cookie> HttpRequest::GetCookie() const {
    return m_cookie_;
}
