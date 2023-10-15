#include "../../include/server/proxy.h"

ProxyServer::ProxyServer(const sys::SocketType& type, 
                         const sys::ThreadMode& thread_mode) {
    this->setType(type);
    this->setThreadType(thread_mode);
}

