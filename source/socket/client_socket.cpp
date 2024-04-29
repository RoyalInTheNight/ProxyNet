#include "../../include/socket/client_socket.h"
#include <arpa/inet.h>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <netinet/in.h>
#include <sys/socket.h>
#include <vector>

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

ClientTypes::SocketStatus SocketClient::socketInit(const std::string& CID) {
    for (auto& _: server)
        if (_.CID == CID) {
            _.socket_ = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

            if (WIN(_.socket_ == INVALID_SOCKET)
                LINUX(_.socket_ < 0))
                return sstatus_t::err_socket_init;

            else break;
        }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::socketConnect(const std::string& CID) {
    for (auto& _: server)
        if (_.CID == CID) {
            if (WIN(_.socket_ == INVALID_SOCKET)
                LINUX(_.socket_ < 0))
                return sstatus_t::err_socket_init;

            int32_t c_result = ::connect(_.socket_, (sockaddr *)&_.header_, _.sosize_);

            if (WIN(c_result == SOCKET_ERROR)
                LINUX(c_result < 0))
                return sstatus_t::err_socket_connect;

            std::vector<char> estabilish_connection = {(char)ESTABILISH_BYTE};

            c_result = ::send(_.socket_, estabilish_connection.data(), 
                                               estabilish_connection.size(), 0);
            
            if (c_result < 0)
                return sstatus_t::err_socket_estabilish;
        }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::sendBy(const std::string& CID, const std::string& data) {
    for (auto& _: server)
        if (_.CID == CID) {
            if (WIN(_.socket_ == INVALID_SOCKET)
                LINUX(_.socket_ < 0))
                return sstatus_t::err_socket_init;

            int32_t c_result = ::send(_.socket_, data.c_str(), data.size(), 0);

            if (c_result < 0)
                return sstatus_t::err_socket_send;
        }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::sendBy(const std::string& CID, const std::vector<char>& data) {
    for (auto& _: server)
        if (_.CID == CID) {
            if (WIN(_.socket_ == INVALID_SOCKET)
                LINUX(_.socket_ < 0))
                return sstatus_t::err_socket_init;

            int32_t c_result = ::send(_.socket_, data.data(), data.size(), 0);

            if (c_result < 0)
                return sstatus_t::err_socket_send;
        }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::sendBy(const std::string& CID, const void *data, const uint32_t size) {
    for (auto& _: server)
        if (_.CID == CID) {
            if (WIN(_.socket_ == INVALID_SOCKET)
                LINUX(_.socket_ < 0))
                return sstatus_t::err_socket_init;

            int32_t c_result = ::send(_.socket_, (char *)data, size, 0);

            if (c_result < 0)
                return sstatus_t::err_socket_send;
        }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::sendAll(const std::string& data) {
    {
        __raw_pool pool(server.size());

        for (auto& _: server) {
            std::function<bool()> __pool = [&]() -> bool {
                if (WIN(_.socket_ == INVALID_SOCKET)
                    LINUX(_.socket_ < 0))
                    return false;

                int32_t c_result = ::send(_.socket_, data.c_str(), data.size(), 0);

                if (c_result < 0)
                    return false;

                return true;
            };

            pool.add_thread(__pool);
        }
    }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::sendAll(const std::vector<char>& data) {
    {
        __raw_pool pool(server.size());

        for (auto& _: server) {
            std::function<bool()> __pool = [&]() -> bool {
                if (WIN(_.socket_ == INVALID_SOCKET)
                    LINUX(_.socket_ < 0))
                    return false;

                int32_t c_result = ::send(_.socket_, data.data(), data.size(), 0);

                if (c_result < 0)
                    return false;

                return true;
            };

            pool.add_thread(__pool);
        }
    }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::sendAll(const void *data, const uint32_t size) {
    {
        __raw_pool pool(server.size());

        for (auto& _: server) {
            std::function<bool()> __pool = [&]() -> bool {
                if (WIN(_.socket_ == INVALID_SOCKET)
                    LINUX(_.socket_ < 0))
                    return false;

                int32_t c_result = ::send(_.socket_, (char *)data, size, 0);

                if (c_result < 0)
                    return false;

                return true;
            };

            pool.add_thread(__pool);
        }
    }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::uploadBy(const std::string& CID, const std::string& path) {
    for (auto& _: server) {
        if (_.CID == CID) {
            if (WIN(_.socket_ == INVALID_SOCKET)
                LINUX(_.socket_ < 0))
                return sstatus_t::err_socket_init;

            if (!std::filesystem::exists(path) || 
                !std::filesystem::is_regular_file(path))
                return sstatus_t::err_socket_upload;

            uint32_t block_size = __INT16_MAX__;
            uint32_t file_size  = std::filesystem::file_size(path) + 1;

            std::vector<char> fbuffer(block_size);

            std::ifstream file(path, std::ios::binary);

            int32_t _loss = 0;

            while (file_size) { // single thread realise
                if (file_size >= block_size) {
                    file.read(fbuffer.data(), block_size);

                    file_size -= block_size;
                }

                else {
                    file.read(fbuffer.data(), (file_size - block_size));

                    file_size = 0;
                }

                _loss = ::send(_.socket_, fbuffer.data(), fbuffer.size(), 0);

                if (_loss < 0) {
                    for (uint32_t i = 0;!_loss && !(i < 5); i++)
                        _loss = ::send(_.socket_, fbuffer.data(), fbuffer.size(), 0);
                
                    if (_loss < 0)
                        return sstatus_t::err_socket_upload_connection_broken;
                }
            }
        }
    }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::uploadBy(const std::string& CID, const std::vector<char>& path) {
    for (auto& _: server) {
        if (_.CID == CID) {
            if (WIN(_.socket_ == INVALID_SOCKET)
                LINUX(_.socket_ < 0))
                return sstatus_t::err_socket_init;

            if (!std::filesystem::exists(path.data()) || 
                !std::filesystem::is_regular_file(path.data()))
                return sstatus_t::err_socket_upload;

            uint32_t block_size = __INT16_MAX__;
            uint32_t file_size  = std::filesystem::file_size(path.data()) + 1;

            std::vector<char> fbuffer(block_size);

            std::ifstream file(path.data(), std::ios::binary);

            int32_t _loss = 0;

            while (file_size) { // single thread realise
                if (file_size >= block_size) {
                    file.read(fbuffer.data(), block_size);

                    file_size -= block_size;
                }

                else {
                    file.read(fbuffer.data(), (file_size - block_size));

                    file_size = 0;
                }

                _loss = ::send(_.socket_, fbuffer.data(), fbuffer.size(), 0);

                if (_loss < 0) {
                    for (uint32_t i = 0;!_loss && !(i < 5); i++)
                        _loss = ::send(_.socket_, fbuffer.data(), fbuffer.size(), 0);
                
                    if (_loss < 0)
                        return sstatus_t::err_socket_upload_connection_broken;
                }
            }
        }
    }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::uploadBy(const std::string& CID, const void *path, const uint32_t size) {
    for (auto& _: server) {
        if (_.CID == CID) {
            if (WIN(_.socket_ == INVALID_SOCKET)
                LINUX(_.socket_ < 0))
                return sstatus_t::err_socket_init;

            char *__path = new char[size];

            memcpy(__path, path, size);

            if (!std::filesystem::exists(__path) || 
                !std::filesystem::is_regular_file(__path))
                return sstatus_t::err_socket_upload;

            uint32_t block_size = __INT16_MAX__;
            uint32_t file_size  = std::filesystem::file_size(__path) + 1;

            std::vector<char> fbuffer(block_size);

            std::ifstream file(__path, std::ios::binary);

            int32_t _loss = 0;

            while (file_size) { // single thread realise
                if (file_size >= block_size) {
                    file.read(fbuffer.data(), block_size);

                    file_size -= block_size;
                }

                else {
                    file.read(fbuffer.data(), (file_size - block_size));

                    file_size = 0;
                }

                _loss = ::send(_.socket_, fbuffer.data(), fbuffer.size(), 0);

                if (_loss < 0) {
                    for (uint32_t i = 0;!_loss && !(i < 5); i++)
                        _loss = ::send(_.socket_, fbuffer.data(), fbuffer.size(), 0);
                
                    if (_loss < 0)
                        return sstatus_t::err_socket_upload_connection_broken;
                }
            }
        }
    }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::uploadAll(const std::string&) {return sstatus_t::err_socket_ok;}
ClientTypes::SocketStatus SocketClient::uploadAll(const std::vector<char>&) {return sstatus_t::err_socket_ok;}
ClientTypes::SocketStatus SocketClient::uploadAll(const void *, const uint32_t) {return sstatus_t::err_socket_ok;}