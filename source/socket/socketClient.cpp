#include "../../include/socket/socket.h"

ProxyNet::Socket::Client::Client(
    SocketData *sda, const std::string& CID, const SSL ssl) {
    this->setPath(".client_" + CID);
    this->setLogging(logging::LoggingStatus::loggingFile);

    this->CID = CID;
    this->ssl = ssl;

    if (sda == nullptr) {
        this->logging("[FAILED]Constructor Client(): SOData = nullptr");

        exit(-0x1);
    }

    this->sodata = new SocketData(
        sda->header.sin_addr.s_addr,
        sda->header.sin_port,
        sda->socket,
        sda->header.sin_family
    );

    this->logging("[  OK  ]Constructor Client(): ok");
}

ProxyNet::Socket::Client::Client(const Client& ClientSDA) {
    this->setPath(".client_" + CID);
    this->setLogging(logging::LoggingStatus::loggingFile);

    this->CID = ClientSDA.CID;

    if (ClientSDA.sodata == nullptr) {
        this->logging("[FAILED]Constructor Client(): SOData = nullptr");

        exit(-0x1);
    }

    this->sodata = new SocketData(
        ClientSDA.sodata->header.sin_addr.s_addr,
        ClientSDA.sodata->header.sin_port,
        ClientSDA.sodata->socket,
        ClientSDA.sodata->header.sin_family
    );

    this->KEY  = ClientSDA.KEY;
    this->flag = ClientSDA.flag;

    this->logging("[  OK  ]Constructor Client(): ok");
}

ProxyNet::SocketData *ProxyNet::Socket::Client::getSOData() const {
    return this->sodata;
}

std::string ProxyNet::Socket::Client::getCID() const {
    return this->CID;
}

void ProxyNet::Socket::Client::setSOData(const uint32_t address, 
                                         const uint16_t port, 
                                         const uint32_t SSL_PKEY, 
                                         const uint8_t  flag) {
    if (this->sodata == nullptr) {
        this->logging("[FAILED]Func setSOData(): SOData = nullptr");

        return;
    }

    this->sodata->header.sin_addr.s_addr = address;
    this->sodata->header.sin_port        = port;

    this->KEY  = SSL_PKEY;
    this->flag = flag;

    this->logging("[  OK  ]Func setSOData(): ok");
}

int32_t ProxyNet::Socket::Client::send(const std::string& message) {
    if (this->sodata == nullptr) {
        this->logging("[FAILED]Func send(): SOData = nullptr");

        return -0x2;
    }

    if (ssl == SSL::SSL_Disable) {
        this->logging("[  OK  ]Func send(): ok");
    
        return ::send(this->sodata->socket, message.c_str(), message.size(), 0);
    }

    if (ssl == SSL::SSL_Enable) {
        this->logging("[  OK  ]Func send(): ok");

        // to dev...
        return 0x0;
    }

    return -0x3;
}

int32_t ProxyNet::Socket::Client::send(const std::vector<char>& message) {
    if (this->sodata == nullptr) {
        this->logging("[FAILED]Func send(): SOData = nullptr");

        return -0x2;
    }

    if (ssl == SSL::SSL_Disable) {
        this->logging("[  OK  ]Func send(): ok");
    
        return ::send(this->sodata->socket, message.data(), message.size(), 0);
    }

    if (ssl == SSL::SSL_Enable) {
        this->logging("[  OK  ]Func send(): ok");

        // to dev...
        return 0x0;
    }

    return -0x3;
}

int32_t ProxyNet::Socket::Client::send(const void *data, const uint32_t size) {
    if (this->sodata == nullptr) {
        this->logging("[FAILED]Func send(): SOData = nullptr");

        return -0x2;
    }

    if (data == nullptr) {
        this->logging("[FAILED]Func send(): data = nullptr");

        return -0x1;
    }

    if (ssl == SSL::SSL_Disable) {
        this->logging("[  OK  ]Func send(): ok");

        return ::send(this->sodata->socket, (char *)data, size, 0);
    }

    if (ssl == SSL::SSL_Enable) {
        this->logging("[  OK  ]Func send(): ok");
     
        // to dev..
        return 0x0;
    }

    return -0x3;
}

int32_t ProxyNet::Socket::Client::read(std::string& message) {
    if (this->sodata == nullptr) {
        this->logging("[FAILED]Func read(): SOData = nullptr");

        return -0x2;
    }

    if (ssl == SSL::SSL_Disable) {
        this->logging("[  OK  ]Func send(): ok");
        
        return ::recv(this->sodata->socket, (char *)message.c_str(), INT16, 0);
    }

    if (ssl == SSL::SSL_Enable) {
        this->logging("[  OK  ]Func send(): ok");

        // to dev..
        return 0x0;
    }

    return -0x3;
}

int32_t ProxyNet::Socket::Client::read(std::vector<char>& message) {
    if (this->sodata == nullptr) {
        this->logging("[FAILED]Func read(): SOData = nullptr");
 
        return -0x2;
    }

    if (ssl == SSL::SSL_Disable) {
        this->logging("[  OK  ]Func read(): ok");

        return ::recv(this->sodata->socket, message.data(), INT16, 0);
    }

    if (ssl == SSL::SSL_Enable) {
        this->logging("[  OK  ]Func read(): ok");

        // to dev..
        return 0x0;
    }

    return -0x3;
}

int32_t ProxyNet::Socket::Client::read(void *data, const uint32_t size) {
    if (this->sodata == nullptr) {
        this->logging("[FAILED]Func read(): SOData = nullptr");

        return -0x2;
    }

    if (data == nullptr) {
        this->logging("[FAILED]Func read(): data = nullptr");
        
        return -0x2;
    }

    if (ssl == SSL::SSL_Disable) {
        this->logging("[  OK  ]Func read(): ok");

        return ::recv(this->sodata->socket, (char *)data, size, 0);
    }

    if (ssl == SSL::SSL_Enable) {
        this->logging("[  OK  ]Func read(): ok");

        // to dev..
        return 0x0;
    }

    return -0x3;
}

#include <unistd.h>

void ProxyNet::Socket::Client::disconnect() { close(this->sodata->socket); }
ProxyNet::Socket::Client::~Client() { this->disconnect(); }