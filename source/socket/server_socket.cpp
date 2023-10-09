//
// Created by MikoG on 07.10.2023.
//

#include "../../include/socket/server_socket.h"

sys::SocketServer::SocketServer() {
    #ifdef _WIN32
        WSADATA wsa;

        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            this->_status = status::err_init_socket;
    #endif // WIN64
}

sys::SocketServer::SocketServer(const uint16_t port) {
    this->port = port;

    #ifdef _WIN32
        WSADATA wsa;

        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            this->_status = status::err_init_socket;
    #endif // WIN64
}

sys::SocketServer::SocketServer(const SocketServer& server) {
    this->_status = std::move(server._status);
    this->_mode   = std::move(server._mode);
    this->_type   = std::move(server._type);

    this->srv_header = std::move(server.srv_header);
    this->listClient = std::move(server.listClient);

    this->srv_socket = server.srv_socket;
    this->port       = server.port;

    #ifdef _WIN32
        WSADATA wsa;

        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            this->_status = status::err_init_socket;
    #endif // WIN64
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

        client.setSocket(cli_socket);
        client.setHeader(cli_header);

        listClient.push_back(client);
    }

    if (this->_mode == mode::multi_thread) {

    }

    return true;
}

std::vector<sys::ClientConnectionData> sys::SocketServer::getClients() const {
    std::vector <ClientConnectionData> clConList;

    ClientConnectionData client;

    for (auto& clCon : listClient) {
        client.host = clCon.getHost();
        client.port = clCon.getPort();

        clConList.push_back(client);
    }

    return clConList;
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
    if (WIN(::send(cli_socket, "f", 1, 0) == SOCKET_ERROR)
        LINUX(::send(cli_socket, "f", 1, 0) < 0))
        return false;

    return true;
}

bool sys::SocketServer::connectBy(const uint32_t host, const uint16_t port) {
    bool   isHostTrue = false;
    bool   isPortTrue = false;
    bool returnResult = false;

    for (auto& clList : listClient) {
        if (clList.getHost() == host)
            isHostTrue = true;

        if (clList.getPort() == port)
            isPortTrue = true;

        if (isHostTrue && isPortTrue) {
            if (clList.isConnected())
                return true;

            returnResult = clList.connectClient();
        }

        isHostTrue = false;
        isPortTrue = false;
    }

    return returnResult;
}

bool sys::SocketServer::disconnectBy(const uint32_t host, const uint16_t port) {
    bool   isHostTrue = false;
    bool   isPortTrue = false;
    bool returnResult = false;

    for (auto& clList : listClient) {
        if (clList.getHost() == host)
            isHostTrue = true;

        if (clList.getPort() == port)
            isPortTrue = true;

        if (isHostTrue && isPortTrue) {
            if (clList.isConnected())
                returnResult = clList.disconnectClient();

            else
                return false;
        }

        isHostTrue = false;
        isPortTrue = false;
    }

    return returnResult;
}

bool sys::SocketServer::sendBy(const uint32_t host, 
                               const uint16_t port, 
                               const void *message,
                               uint32_t size) {
    bool isHostTrue   = false;
    bool isPortTrue   = false;
    bool returnResult = false;

    for (auto& clList : listClient) {
        if (clList.getHost() == host)
            isHostTrue = true;

        if (clList.getPort() == port)
            isPortTrue = true;

        if (isHostTrue && isPortTrue)
            returnResult = clList.sendClientData((void *)message, size);

        isHostTrue = false;
        isPortTrue = false;
    }

    return returnResult;
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
    }

    if (!this->connectClient())
        return false;

    result = ::send(cli_socket, message.c_str(), message.size(), 0);

    if (WIN(result == SOCKET_ERROR)LINUX(result < 0))
        return false;

    return true;
}