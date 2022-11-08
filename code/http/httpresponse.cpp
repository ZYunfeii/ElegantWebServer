/*
 * @Author       : mark
 * @Date         : 2020-06-27
 * @copyleft Apache 2.0
 */ 
#include "httpresponse.h"

using namespace std;


const unordered_map<string, string> HttpResponse::SUFFIX_TYPE = {
    { ".html",  "text/html" },
    { ".xml",   "text/xml" },
    { ".xhtml", "application/xhtml+xml" },
    { ".txt",   "text/plain" },
    { ".rtf",   "application/rtf" },
    { ".pdf",   "application/pdf" },
    { ".word",  "application/nsword" },
    { ".png",   "image/png" },
    { ".gif",   "image/gif" },
    { ".jpg",   "image/jpeg" },
    { ".jpeg",  "image/jpeg" },
    { ".au",    "audio/basic" },
    { ".mpeg",  "video/mpeg" },
    { ".mpg",   "video/mpeg" },
    { ".avi",   "video/x-msvideo" },
    { ".gz",    "application/x-gzip" },
    { ".tar",   "application/x-tar" },
    { ".css",   "text/css "},
    { ".js",    "text/javascript "},
};

const unordered_map<int, string> HttpResponse::CODE_STATUS = {
    { 200, "OK" },
    { 400, "Bad Request" },
    { 403, "Forbidden" },
    { 404, "Not Found" },
};

const unordered_map<int, string> HttpResponse::CODE_PATH = {
    { 400, "/400.html" },
    { 403, "/403.html" },
    { 404, "/404.html" },
};

const unordered_set<string> HttpResponse::STRING_GET = {
    "/get-num-visits", 
};

HttpResponse::HttpResponse() {
    code_ = -1;
    path_ = srcDir_ = "";
    isKeepAlive_ = false;
    mmFile_ = nullptr; 
    mmFileStat_ = { 0 };
    hitRedisTag_ = false;
    redisFile_ = "";
};

HttpResponse::~HttpResponse() {
    UnmapFile();
}

void HttpResponse::Init(const string& srcDir, string& path, std::shared_ptr<cookie> cke, bool isKeepAlive, int code){
    assert(srcDir != "");
    if(mmFile_) { UnmapFile(); }
    code_ = code;
    isKeepAlive_ = isKeepAlive;
    path_ = path;
    srcDir_ = srcDir;
    mmFile_ = nullptr; 
    mmFileStat_ = { 0 };
    mCookie_ = cke;
    if (STRING_GET.find(path_) != STRING_GET.end()) ifTransNotFile_ = true; // 非文件请求
    else ifTransNotFile_ = false;
    redis_ = RedisCache::Instance();
}

void HttpResponse::MakeResponse(Buffer& buff) {
    hitRedisTag_ = redis_->existKey(path_);

    /* 判断请求的资源文件 命中Redis就不要去开文件了*/
    if (!ifTransNotFile_ && !hitRedisTag_){
        if(stat((srcDir_ + path_).data(), &mmFileStat_) < 0 || S_ISDIR(mmFileStat_.st_mode)) { // stat通过文件名filename获取文件信息，并保存在mmFileStat_中(其中有size等信息)
            code_ = 404;
        }
        else if(!(mmFileStat_.st_mode & S_IROTH)) {
            code_ = 403;
        }
    } 
    ErrorHtml_();
    AddStateLine_(buff);
    AddHeader_(buff);
    AddContent_(buff);
}

char* HttpResponse::File() {
    return mmFile_;
}

size_t HttpResponse::FileLen() const {
    return mmFileStat_.st_size;
}

void HttpResponse::ErrorHtml_() {
    if(CODE_PATH.count(code_) == 1) {
        path_ = CODE_PATH.find(code_)->second;
        stat((srcDir_ + path_).data(), &mmFileStat_);
    }
}

void HttpResponse::AddStateLine_(Buffer& buff) {
    string status;
    if(CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    }
    else {
        code_ = 400;
        status = CODE_STATUS.find(400)->second;
    }
    buff.Append("HTTP/1.1 " + to_string(code_) + " " + status + "\r\n");
}

void HttpResponse::AddHeader_(Buffer& buff) {
    buff.Append("Connection: ");
    if(isKeepAlive_) {
        buff.Append("keep-alive\r\n");
        buff.Append("keep-alive: max=6, timeout=120\r\n"); // 保持连接最少timeouts 在连接关闭之前，在此连接可以发送的请求的最大值为max
    } else{
        buff.Append("close\r\n");
    }
    buff.Append("Content-type: " + GetFileType_() + "\r\n"); 
    if(mCookie_->isCookie_) {
        buff.Append("Set-Cookie:sessionid=" + (mCookie_->sendCookieStr_) + ";HttpOnly; Path=/" + "\r\n");
    }
}

void HttpResponse::AddContent_(Buffer& buff) {
    // 若命中Redis缓存，从中直接取出返回
    if (hitRedisTag_) {
        redisFile_ = redis_->getKeyVal(path_);
        mmFileStat_.st_size = redisFile_.length();
        mmFile_ = const_cast<char*>(redisFile_.data());
        buff.Append("Content-length: " + to_string(mmFileStat_.st_size) + "\r\n\r\n");
        LOG_INFO("Return redis cache: %s", path_.data());
        return;
    }
    // 若没有命中Redis缓存
    if (!ifTransNotFile_) {
        int srcFd = open((srcDir_ + path_).data(), O_RDONLY);
        if(srcFd < 0) { 
            ErrorContent(buff, "File NotFound!");
            return; 
        }

        /* 将文件映射到内存提高文件的访问速度 
            MAP_PRIVATE 建立一个写入时拷贝的私有映射*/
        LOG_DEBUG("file path %s", (srcDir_ + path_).data());
        int* mmRet = (int*)mmap(0, mmFileStat_.st_size, PROT_READ, MAP_PRIVATE, srcFd, 0);
        if(*mmRet == -1) {
            ErrorContent(buff, "File NotFound!");
            return; 
        }
        mmFile_ = (char*)mmRet;
        close(srcFd);
        buff.Append("Content-length: " + to_string(mmFileStat_.st_size) + "\r\n\r\n");
        string tmp(mmFile_, mmFile_ + mmFileStat_.st_size);
        if (!redis_->setKeyVal(path_, tmp)) {
            LOG_DEBUG("Set key error!");
        }
        return;
    }    
    // ToDo: 传输非文件 
}

void HttpResponse::UnmapFile() {
    if(mmFile_) {
        munmap(mmFile_, mmFileStat_.st_size);
        mmFile_ = nullptr;
    }
}

string HttpResponse::GetFileType_() {
    /* 判断文件类型 */
    string::size_type idx = path_.find_last_of('.');
    if(idx == string::npos) {
        return "text/plain";
    }
    string suffix = path_.substr(idx);
    if(SUFFIX_TYPE.count(suffix) == 1) {
        return SUFFIX_TYPE.find(suffix)->second;
    }
    return "text/plain";
}

void HttpResponse::ErrorContent(Buffer& buff, string message) 
{
    string body;
    string status;
    body += "<html><title>Error</title>";
    body += "<body bgcolor=\"ffffff\">";
    if(CODE_STATUS.count(code_) == 1) {
        status = CODE_STATUS.find(code_)->second;
    } else {
        status = "Bad Request";
    }
    body += to_string(code_) + " : " + status  + "\n";
    body += "<p>" + message + "</p>";
    body += "<hr><em>TinyWebServer</em></body></html>";

    buff.Append("Content-length: " + to_string(body.size()) + "\r\n\r\n");
    buff.Append(body);
}

bool HttpResponse::ifTransNotFile() {
    return ifTransNotFile_;
}