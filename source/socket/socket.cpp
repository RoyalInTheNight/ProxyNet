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

void ProxyNet::Socket::setThreadStatus(const ThreadStatus& thread_) { this->thread = thread_; }
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

        return LSAData::SocketListen;
    }

    this->logging("[  OK  ]Func socketInit(): listen ok");

    return LSAData::SocketOK;
}

#include <ctime>
#include "../../include/thread/thread.h"

int32_t ProxyNet::Socket::socketAccept() {
    if (this->sodata == nullptr) {
        this->logging("[FAILED]Func socketAccept(): SOData = nullptr");

        return LSAData::SocketSOData;
    }

    this->logging("[  OK  ]Func socketAccept(): SOData ok");

    if (this->sodata->socket < 0) {
        this->logging("[FAILED]Func socketAccept(): invalid socket");

        return LSAData::Socket;
    }

    this->logging("[  OK  ]Func socketAccept(): socket ok");

    if (this->thread == ThreadStatus::threadDisable) {
        ESData ESD;
        sha256 sha;

        srand(
            (
                unsigned
            )
            
            time(
                NULL
            )
        );

        sha.update(
            std::to_string(
                std::rand()
            )
        );

        socket_t accept_socket = ::accept(
            this->sodata->socket,
            nullptr,
            nullptr
        );

        if (accept_socket < 0) {
            this->logging("[FAILED]Func socketAccept(): accept failed");

            return LSAData::SocketAccept;
        }

        list.push_back(
            Client(
                new SocketData(
                    accept_socket,
                    AF_INET
                ),

                sha256::to_string(sha.digest()),
                this->ssl
            )
        );

        /*if (
            list[
                clientCount
            ].getSOData()->socket < 0
        ) {
            this->logging("[FAILED]Func socketAccept(): accept failed");

            return LSAData::SocketAccept;
        }*/

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

        this->clientCount++;

        return LSAData::SocketOK;
    }

    if (this->thread == ThreadStatus::threadEnable) {
        pool<std::function<bool()>, bool> TPool(
            std::thread::hardware_concurrency()
        );

        for (uint32_t i = 0; i < std::thread::hardware_concurrency(); i++) {
            std::function<bool()> pool_emp = [&]() -> bool {
                ESData ESD;
                sha256 sha;

                srand(
                    (
                        unsigned
                    )
                    
                    time(
                        NULL
                    )
                );

                sha.update(
                    std::to_string(
                        std::rand()
                    )
                );

                socket_t accept_socket = ::accept(
                    this->sodata->socket,
                    nullptr,
                    nullptr
                );

                if (accept_socket < 0) {
                    this->logging("[FAILED]Func socketAccept(): accept failed");

                    return LSAData::SocketAccept;
                }

                list.push_back(
                    Client(
                        new SocketData(
                            accept_socket,
                            AF_INET
                        ),

                        sha256::to_string(sha.digest()),
                        this->ssl
                    )
                );

                /*if (
                    list[
                        clientCount
                    ].getSOData()->socket < 0
                ) {
                    this->logging("[FAILED]Func socketAccept(): accept failed");

                    return LSAData::SocketAccept;
                }*/

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

                this->clientCount++;

                return LSAData::SocketOK;
            };

            TPool.add_thread(pool_emp);
        } 

        TPool.join();

        for (const auto& _: TPool.pool_thread_result())
            if (_ != true)
                this->logging("[FAILED]Thread fail");
    }

    return LSAData::SocketOK;
}

int32_t ProxyNet::Socket::sendTo(const std::string& CID, const std::string& message) {
    for (auto& _: list)
        if (_.getCID() == CID)
            return _.send(message);

    return -0x1;
}

int32_t ProxyNet::Socket::sendTo(const std::string& CID, const std::vector<char>& message) {
    for (auto& _: list)
        if (_.getCID() == CID)
            return _.send(message);

    return -0x1;
}

int32_t ProxyNet::Socket::sendTo(const std::string& CID, const void *data, const uint32_t size) {
    if (data == nullptr)
        return -0x2;
    
    for (auto& _: list)
        if (_.getCID() == CID)
            return _.send(data, size);

    return -0x1;
}

int32_t ProxyNet::Socket::sendAll(const std::string& message) {
    for (auto& _: list)
        if (!_.send(message))
            return -0x1;

    return 0x1;
}

int32_t ProxyNet::Socket::sendAll(const std::vector<char>& message) {
    for (auto& _: list)
        if (!_.send(message))
            return -0x1;

    return 0x1;
}

int32_t ProxyNet::Socket::sendAll(const void *data, const uint32_t size) {
    if (data == nullptr)
        return -0x2;

    for (auto& _: list)
        if (!_.send(data, size))
            return -0x1;

    return 0x1;
}

int32_t ProxyNet::Socket::readTo(const std::string& CID, std::string& message) {
    for (auto& _: list)
        if (_.getCID() == CID)
            return _.read(message);

    return -0x1;
}

int32_t ProxyNet::Socket::readTo(const std::string& CID, std::vector<char>& message) {
    for (auto& _: list)
        if (_.getCID() == CID)
            return _.read(message);

    return -0x1;
}

int32_t ProxyNet::Socket::readTo(const std::string& CID, void *data, const uint32_t size) {
    if (data == nullptr)
        return -0x2;
    
    for (auto& _: list)
        if (_.getCID() == CID)
            return _.read(data, size);

    return -0x1;
}

void ProxyNet::Socket::disconnectTo(const std::string& CID) {
    for (auto& _: list)
        if (_.getCID() == CID)
            _.disconnect();
}

void ProxyNet::Socket::disconnectAll() {
    for (auto& _: list)
        _.disconnect();
}

ProxyNet::Socket::~Socket() { this->disconnectAll(); }