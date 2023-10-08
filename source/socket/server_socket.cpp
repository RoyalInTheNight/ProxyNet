//
// Created by MikoG on 07.10.2023.
//

#include "../../include/socket/server_socket.h"

sys::SocketServer::SocketServer() {
    #ifdef WIN64
        WSADATA wsa;

        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            this->_status = status::err_init_socket;
    #endif // WIN64
}

sys::SocketServer::SocketServer(const uint16_t port) {
    this->port = port;

    #ifdef WIN64
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

    #ifdef WIN64
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
    }

    if (this->_type == type::udp_socket) {
        cli_socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (WIN(cli_socket == INVALID_SOCKET)
            LINUX(cli_socket < 0))
            return false;
    }

    int32_t result = ::connect(cli_socket, (const sockaddr *)&cli_header, sizeof(cli_header));

    if (WIN(result == SOCKET_ERROR)
        LINUX(result < 0))
        return false;
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

        if (isHostTrue && isPortTrue)
            returnResult = clList.connectClient();

        isHostTrue = false;
        isPortTrue = false;
    }

    return returnResult;
}