#include "../../include/socket/client_socket.h"
#include <arpa/inet.h>
#include <sys/socket.h>

SocketClient::SocketClient(const std::string& address, const uint16_t port) {
    ClientTypes::ServerConnectionData server_connection;

    server_connection.address = address;
    server_connection.port    = port;
    server_connection.CID     = sha256::hash(address, 
                                sha256::sha256_options::option_string_hash);

    server_connection.header_.sin_addr.s_addr = inet_addr(address.c_str());
    server_connection.header_.sin_port        = htons(port);
    server_connection.header_.sin_family      = AF_INET;
    server_connection.sosize_                 = sizeof(server_connection.header_);

    server.push_back(server_connection);
}

SocketClient::SocketClient(const SocketClient& socketClient) {
    server   = socketClient.server;
    sstatus_ = socketClient.sstatus_;
    tstatus_ = socketClient.tstatus_;
}

SocketClient::SocketClient() {}

void SocketClient::setConnectionData(const std::string& address, const uint16_t port) {
    ClientTypes::ServerConnectionData server_connection;

    server_connection.address = address;
    server_connection.port    = port;
    server_connection.CID     = sha256::hash(address,
                                sha256::sha256_options::option_string_hash);

    server_connection.header_.sin_addr.s_addr = inet_addr(address.c_str());
    server_connection.header_.sin_port        = htons(port);
    server_connection.header_.sin_family      = AF_INET;
    server_connection.sosize_                 = sizeof(server_connection.header_);

    server.push_back(server_connection);
}

void SocketClient::setConnectionData(const ClientTypes::ServerConnectionData& scd) {
    if (scd.CID.size() == 0)
        return;

    else if (scd.address.size() == 0 || 
             scd.address.size() > 15)
        return;

    else if (scd.port < 0 || 
             scd.port > __UINT16_MAX__)
        return;

    else
        server.push_back(scd);
}

void SocketClient::setConnectionData(const ClientTypes::header_t& header) {
    if (header.sin_addr.s_addr == 0 ||
        header.sin_addr.s_addr > __UINT32_MAX__)
        return;

    else if (header.sin_port == 0 ||
             header.sin_port > __UINT16_MAX__)
             return;

    else if (header.sin_family != AF_INET)
        return;

    else {
        ClientTypes::ServerConnectionData server_connection;

        server_connection.address = inet_ntoa(header.sin_addr);
        server_connection.port    = htons(header.sin_port);
        server_connection.header_ = header;
        server_connection.sosize_ = sizeof(header);
        server_connection.CID     = sha256::hash(server_connection.address,
                                    sha256::sha256_options::option_string_hash);

        server.push_back(server_connection);
    }
}

void SocketClient::setThreadType(const tstatus_t& tstatus) {
    tstatus_ = tstatus;
}
