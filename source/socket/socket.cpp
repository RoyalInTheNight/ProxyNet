#include "../../include/socket/socket.h"
#include <sys/socket.h>
#include <vector>

using namespace ProxyNet;

Socket::Socket(SocketData *data, const std::string& path, const logging::LoggingStatus& log) {
    this->setPath(path);
    this->setLogging(log);

    if (data == nullptr)
        this->logging("[FAILED]Socket(): SocketData = nullptr");

    else {
        sodata = new SocketData(data->header.sin_addr.s_addr, 
                                   data->header.sin_port, 
                                data->socket, 
                              data->header.sin_family);

        this->logging("[  OK  ]Socket(): success");
    }
}

Socket::Socket(const Socket& data, const std::string& path, const logging::LoggingStatus& log) {
    this->setPath(path);
    this->setLogging(log);

    if (data.sodata == nullptr)
        this->logging("[FAILED]Socket(): SocketData = nullptr");

    else {
        sodata = new SocketData(data.sodata->header.sin_addr.s_addr,
                                   data.sodata->header.sin_port,
                                data.sodata->socket,
                              data.sodata->header.sin_family);

        thread = data.thread;
        list   = data.list;

        this->logging("[  OK  ]Socket(): success");
    }
}

Socket::Socket(const std::string& path, const logging::LoggingStatus& log) {
    this->setPath(path);
    this->setLogging(log);

    this->logging("[ INFO ]Socket(): constructor success");
}

SocketData *           Socket::getSocketData()    const { return this->sodata;       }
ThreadStatus           Socket::getThreadStatus()  const { return this->thread;       }
logging::LoggingStatus Socket::getLoggingStatus() const { return this->getLogging(); }
Socket::cli_t          Socket::getClientList()    const { return this->list;         }

void Socket::setSocketData(SocketData *data) {
    if (data == nullptr)
        this->logging("[FAILED]setSocketData(): SocketData = nullptr");

    else {
        sodata = new SocketData(data->header.sin_addr.s_addr,
                                   data->header.sin_port,
                                data->socket,
                              data->header.sin_family);

        this->logging("[  OK  ]Socket(): success");
    }
}

void Socket::setThreadStatus(const ThreadStatus& thread_) { this->thread = thread_; }
void Socket::setLoggingStatus(const logging::LoggingStatus& log_) { this->setLogging(log_); }
void Socket::setLoggingPath(const std::string& path_) { this->setPath(path_); }

int32_t Socket::socketInit() {
    if (sodata == nullptr) {
        this->logging("[FAILED]socketInit(): SocketData = nullptr");

        return LSAData::SocketSOData;   
    }

    if (sodata->socket < 0) {
        this->logging("[FAILED]socketInit(): socket error");

        return LSAData::Socket;
    }

    else
        this->logging("[  OK  ]socketInit(): socket success");

    int32_t result = ::bind(sodata->socket, (sockaddr *)&sodata->header, sodata->headerSize);

    if (result < 0) {
        this->logging("[FAILED]socketInit(): bind error");

        return LSAData::SocketBind;
    }

    else
        this->logging("[  OK  ]socketInit(): bind success");

    result = ::listen(sodata->socket, 1024);

    if (result < 0) {
        this->logging("[FAILED]socketInit(): listen error");

        return LSAData::SocketListen;
    }

    else
        this->logging("[  OK ]socketInit(): listen success");

    return 0x1;
}

