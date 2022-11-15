/*
 * @Author       : mark
 * @Date         : 2020-06-18
 * @copyleft Apache 2.0
 */ 
// 这份代码实在是Elegant，读起来真的是颅内高潮。
#include <unistd.h>
#include "server/webserver.h"

int main() {
    /* 守护进程 后台运行 */
    //daemon(1, 0); 

    WebServer server(
        9006, 3, 60000, false,               /* 端口 ET模式 timeoutMs 优雅退出  */
        8, true, 1, 1024,                /* 线程池数量 日志开关 日志等级 日志异步队列容量 */
        "127.0.0.1", 6379, 12);                  /* Redis配置 */         
    server.Start();
} 
  