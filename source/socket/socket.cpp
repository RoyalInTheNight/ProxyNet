#include "../../include/socket/socket.h"
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <vector>

ProxyNet::Socket::Socket(ProxyNet::SocketData *sda, const std::string& path, const logging::LoggingStatus& log) {
    this->setPath(path);
    this->setLogging(log);
    
    if (sda == nullptr) {
        this->logging("[FAILED]Constructor Socket(): SOData broken");

        exit(-0x1);
    }

    this->sodata = new SocketData(
        sda->header.sin_addr.s_addr, 
        sda->header.sin_port, 
        sda->socket, 
        sda->header.sin_family
    );

    this->logging("[  OK  ]Constructor Socket(): SOData configuration success");
}

ProxyNet::Socket::Socket(const ProxyNet::Socket& sda, const std::string& path, const logging::LoggingStatus& log) {
    this->setPath(path);
    this->setLogging(log);

    if (sda.sodata == nullptr) {
        this->logging("[FAILED]Constructor Socket(): SOData broken");

        exit(-0x1);
    }

    this->sodata = new SocketData(
        sda.sodata->header.sin_addr.s_addr,
        sda.sodata->header.sin_port,
        sda.sodata->socket,
        sda.sodata->header.sin_family
    );

    this->ssl    = sda.ssl;
    this->thread = sda.thread;
    this->list   = sda.list;

    this->logging("[  OK  ]Constructor Socket(): copy Socket object success");
}

ProxyNet::Socket::Socket(const std::string& path, const logging::LoggingStatus& log) {
    this->setPath(path);
    this->setLogging(log);
    this->logging("[ INFO ]Constructor Socket(): logging configuration success");
}

ProxyNet::SocketData * ProxyNet::Socket::getSocketData()        const { return this->sodata;       }
ProxyNet::ThreadStatus ProxyNet::Socket::getThreadStatus()      const { return this->thread;       }
logging::LoggingStatus ProxyNet::Socket::getLoggingStatus()     const { return this->getLogging(); }
std::vector<ProxyNet::Socket::Client> ProxyNet::Socket::getClientList() const { return this->list; }

void ProxyNet::Socket::setSocketData(ProxyNet::SocketData *sda) {
    if (sda == nullptr) {
        this->logging("[FAILED]Func setSocketData(): SOData = nullptr");

        exit(-0x1);
    }

    this->sodata = new SocketData(
        sda->header.sin_addr.s_addr,
        sda->header.sin_port,
        sda->socket,
        sda->header.sin_family
    );

    this->logging("[  OK  ]Func setSocketData(): SOData configuration success");
}

void ProxyNet::Socket::setThreadStatus(const ThreadStatus& thread) { this->thread = thread; }
void ProxyNet::Socket::setLoggingStatus(const logging::LoggingStatus& log) { this->setLogging(log); }
void ProxyNet::Socket::setLoggingPath(const std::string& path) { this->setPath(path); }

int32_t ProxyNet::Socket::socketInit() {
    if (this->sodata == nullptr) {
        this->logging("[FAILED]Func socketInit(): SOData = nullptr");

        return LSAData::SocketSOData;
    }

    if (this->sodata->socket < 0) {
        this->logging("[FAILED]Func socketInit(): invalid socket");

        return LSAData::Socket;
    }

    this->logging("[  OK  ]Func socketInit(): socket ok");

    int32_t result = ::bind(
        this->sodata->socket, 
        (
            sockaddr *
        )&this->sodata->header, 
        
        this->sodata->headerSize
    );

    if (result < 0) {
        this->logging("[FAILED]Func socketInit(): bind failed");

        return LSAData::SocketBind;
    }

    this->logging("[  OK  ]Func socketInit(): bind ok");

    result = ::listen(
        this->sodata->socket, 
        ProxyNet::listen_queue
    );

    if (result < 0) {
        this->logging("[FAILED]Func socketInit(): listen failed");

        return LSAData::SocketBind;
    }

    this->logging("[  OK  ]Func socketInit(): listen ok");

    return 0x0;
}

int32_t ProxyNet::Socket::socketAccept() {
    if (this->sodata == nullptr) {
        this->logging("[FAILED]Func socketAccept(): SOData = nullptr");

        return LSAData::SocketSOData;
    }

    if (this->sodata->socket < 0) {
        this->logging("[  OK  ]Func socketAccept(): invalid socket");

        return LSAData::Socket;
    }

    if (this->thread == ThreadStatus::threadDisable) {
        ESData ESD;

        list.push_back(
            Client(
                new SocketData(
                    ::accept(
                        this->sodata->socket, 
                        nullptr, 
                        nullptr
                    ),

                    AF_INET
                ),

                1 // CID to dev...
            )
        );

        if (
            list[
                clientCount
            ].getSOData()->socket < 0
        ) {
            this->logging("[FAILED]Func socketAccept(): accept failed");

            return LSAData::SocketAccept;
        }

        this->logging("[  OK  ]Func socketAccept(): accept ok");

        // estabilish connection
        this->logging("[ INFO ]Func socketAccept(): estabilish");

        char *estabilish_buffer = new char[
            sizeof(
                ESData
            )
        ];

        int32_t bytes_read = ::recv(
            list[
                clientCount
            ].getSOData()->socket,

            estabilish_buffer,
            sizeof(
                ESData
            ),

            0
        );

        if (bytes_read < 0) {
            this->logging("[FAILED]Func socketAccept(): estabilish failed");

            return LSAData::SocketEstabilish;
        }

        this->logging("[  OK  ]Func socketAccept(): estabilish ok");

        memcpy(
            &ESD, 
            estabilish_buffer, 
            sizeof(
                ESData
            )
        );

        list[
            clientCount
        ].setSOData(
            ESD.address, 
            ESD.port, 
            ESD.SSL_PKEY, 
            ESD.flag
        );

        return 0x1;
    }

    if (this->thread == ThreadStatus::threadEnable) {

    }

    return -0x2;
}