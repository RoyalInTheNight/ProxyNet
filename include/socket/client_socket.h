#ifndef __CLIENT_SOCKET
#define __CLIENT_SOCKET

#ifdef _WIN32
#include <winsock2.h>
#include <ws2tcpip.h>

#define WIN(exp) exp
#define LINUX(exp)
#else
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>

#define WIN(exp)
#define LINUX(exp) exp
#endif // _WIN32

#include <vector>
#include <string>

#include <functional>
#include <cstdint>

#include "../../include/thread_pool/pool.h"
#include "../../include/crypto/sha256.h"

#define PROXY_MESSAGE     0xff
#define ESTABILISH_BYTE   0xfe
#define SHELL_MODE_BYTE   0xfd
#define UPDATE_MODE_BYTE  0xfc
#define KEEP_ALIVE_PING   0xfb
#define CLOUD_CLIENT_BYTE 0xee
#define PROXY_MODE_FAILED 0xef
#define IS_CONNECTED      0xed

namespace ClientTypes {
    #ifdef _WIN32
        typedef SOCKET      socket_t;
        typedef sockaddr_in header_t;
        typedef int32_t     sosize_t;
    #else
        typedef int32_t     socket_t;
        typedef sockaddr_in header_t;
        typedef socklen_t   sosize_t;
    #endif // _WIN32

    enum class SocketStatus : uint8_t {
        err_socket_init                     = 0x1,
        err_socket_connect                  = 0x2,
        err_socket_fsend                    = 0x3,
        err_socket_frecv                    = 0x4,
        err_socket_send                     = 0x5,
        err_socket_recv                     = 0x6,
        err_socket_estabilish               = 0x7,
        err_socket_upload                   = 0x8,
        err_socket_upload_connection_broken = 0x9,
        err_socket_ok                       = 0x0
    };

    enum class ThreadStatus : uint8_t {
        mthread_status = 0xfe,
        sthread_status = 0xff
    };

    struct ServerConnectionData {
        ClientTypes::header_t header_;
        ClientTypes::socket_t socket_;
        ClientTypes::sosize_t sosize_;

        std::string     CID;
        std::string address;
        uint16_t       port;
    };

    namespace bit_handler {
        static bool upload_mode;
        static bool  shell_mode;
        static bool   ping_mode;
    }

    namespace chr_handler {
        const char shell_mode   = SHELL_MODE_BYTE;
        const char upload_mode  = UPDATE_MODE_BYTE;
        const char ping_mode    = KEEP_ALIVE_PING;
        const char is_connected = IS_CONNECTED;
    }
}

class SocketClient {
private:
    typedef ClientTypes::SocketStatus sstatus_t;
    typedef ClientTypes::ThreadStatus tstatus_t;

    sstatus_t sstatus_;
    tstatus_t tstatus_;

    std::vector<ClientTypes::ServerConnectionData> server;

public:
    SocketClient(const std::string&, const uint16_t);
    SocketClient(const SocketClient&);
    SocketClient();

    [[nodiscard]] ClientTypes::socket_t getClientSocket(const std::string&)   const;
    [[nodiscard]] ClientTypes::header_t getClientHeader(const std::string&)   const;
    [[nodiscard]] std::string           getServerAddress(const std::string&)  const;
    [[nodiscard]] uint16_t              getServerPort(const std::string&)     const;
    [[nodiscard]] sstatus_t             getClientSocketStatus()               const;
    [[nodiscard]] tstatus_t             getClientThreadStatus()               const;
    [[nodiscard]] std::vector<ClientTypes::ServerConnectionData> 
                                        getServerConnectionData()             const {return server;}

    void setConnectionData(const std::string&, const uint16_t);
    void setConnectionData(const ClientTypes::ServerConnectionData&);
    void setConnectionData(const ClientTypes::header_t&);
    void setThreadType(const tstatus_t&);

    sstatus_t socketInit(const std::string&);
    sstatus_t socketConnect(const std::string&);

    sstatus_t sendBy(const std::string&, const std::string&);
    sstatus_t sendBy(const std::string&, const std::vector<char>&);
    sstatus_t sendBy(const std::string&, const void *, const uint32_t);

    sstatus_t sendAll(const std::string&);
    sstatus_t sendAll(const std::vector<char>&);
    sstatus_t sendAll(const void *, const uint32_t);

    sstatus_t uploadBy(const std::string&, const std::string&);
    sstatus_t uploadBy(const std::string&, const std::vector<char>&);
    sstatus_t uploadBy(const std::string&, const void *, const uint32_t);

    sstatus_t uploadAll(const std::string&);
    sstatus_t uploadAll(const std::vector<char>&);
    sstatus_t uploadAll(const void *, const uint32_t);

    sstatus_t downloadBy(const std::string&, const std::string&);
    sstatus_t downloadBy(const std::string&, const std::vector<char>&);
    sstatus_t downloadBy(const std::string&, const void *, const uint32_t);

    sstatus_t recvHandler();

    void removeServer(const std::string&);
    void removeClient();

    bool disconnectBy(const std::string&);

    uint32_t serverActivity();
    uint32_t serverList();
};

#endif // __CLIENT_SOCKET
