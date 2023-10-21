#ifndef SRV_SOCKET
#define SRV_SOCKET

#include <cstdint>
#ifdef _WIN32
#  include <winsock2.h>
#  include <ws2tcpip.h>

#  define WIN(exp) exp
#  define LINUX(exp)
#else
#  include <sys/socket.h>
#  include <sys/types.h>
#  include <arpa/inet.h>
#  include <netinet/in.h>
#  include <unistd.h>

#  define WIN(exp)
#  define LINUX(exp) exp
#endif // WIN64

#include <vector>
#include <string>
#include <thread>

#include <functional>

#include "../../include/crypto/sha256.h"

namespace sys {
    enum class SocketStatus : uint8_t {
        up = 0,
        connected = 1,
        disconnected = 2,
        err_init_socket = 3,
        err_bind_socket = 4,
        err_connection_socket = 5
    };

    enum class SocketType : uint8_t {
        tcp_socket = 0xfe,
        udp_socket = 0xff
    };

    enum class ThreadMode : uint32_t {
        single_thread = 0xfffe,
        multi_thread  = 0xffff
    };

    struct ClientConnectionData {
        uint32_t host;
        uint16_t port;
    } __attribute__((packed));

    class SocketServer {
    protected:
        struct Client;

        typedef SocketStatus                 status;
        typedef SocketType                     type;
        typedef ThreadMode                     mode;
        typedef std::function<void()> recvHandler_t;

        status _status;
        type     _type;
        mode     _mode;

        #ifdef _WIN32
            typedef SOCKET      Socket_t;
            typedef sockaddr_in SockIn_t;
            typedef int32_t    SockLen_t;
        #else
            typedef int32_t     Socket_t;
            typedef sockaddr_in SockIn_t;
            typedef socklen_t  SockLen_t;
        #endif // WIN64

        Socket_t srv_socket;
        SockIn_t srv_header;

        uint16_t port;

        std::vector<Client>   listClient;

    public:
        SocketServer();
        SocketServer(const uint16_t);
        SocketServer(const SocketServer&);

        [[nodiscard]] Socket_t                     getServerSocket() const { return this->srv_socket; }
        [[nodiscard]] SockIn_t                     getServerHeader() const { return this->srv_header; }
        [[nodiscard]] uint32_t                             getHost() const { return 0x00000000;       }
        [[nodiscard]] uint16_t                             getPort() const { return this->port;       }
        [[nodiscard]] ThreadMode                     getThreadMode() const { return this->_mode;      }
        [[nodiscard]] SocketType                     getSocketType() const { return this->_type;      }
        [[nodiscard]] SocketStatus                 getSocketStatus() const { return this->_status;    }
        [[nodiscard]] std::vector<ClientConnectionData> getClients() const;
        [[nodiscard]] std::vector<std::string>              getCID();

        void setPort(const uint16_t server_port)          { this-> port = server_port; }
        void setType(const SocketType& socket_type)       { this->_type = socket_type; }
        void setThreadType(const ThreadMode& thread_mode) { this->_mode = thread_mode; }

        bool socketInit();
        bool socketListenConnection();
        bool connectBy(const std::string&);

        bool sendBy(const std::string&, const void *, uint32_t);
        bool sendBy(const std::string&, const std::vector<char>&);
        bool sendBy(const std::string&, const std::string&);

        std::string readClientData(const std::string&);

        uint64_t sendAll(const void *, uint32_t);
        uint64_t sendAll(const std::vector<char>&);
        uint64_t sendAll(const std::string&);

        bool disconnectBy(const std::string&);
        bool disconnectAll();

        uint64_t checkBot();
        uint64_t sizeListBot();

        ~SocketServer();
    };

    struct SocketServer::Client {
    private:
        Socket_t   cli_socket;
        SockIn_t   cli_header;

        type            _type;

        std::vector<char>
                     cli_data;
        sha256        sha_imp;
        
    public:
        [[nodiscard]] std::vector<char>   getData() const { return this->cli_data;   }
        [[nodiscard]] Socket_t          getSocket() const { return this->cli_socket; }
        [[nodiscard]] SockIn_t          getHeader() const { return this->cli_header; }
        [[nodiscard]] uint16_t            getPort() const { return htons(this->cli_header.sin_port); }
        [[nodiscard]] uint32_t            getHost() const { return this->cli_header WIN(.sin_addr.S_un.S_addr)LINUX(.sin_addr.s_addr); }
        [[nodiscard]] std::vector<char>    getCID() const {
            std::string hash = sha_imp.hash(inet_ntoa(cli_header.sin_addr), sha256::sha256_options::option_string_hash);
            std::vector<char> CID;

            for (const auto& symbol : hash)
                CID.push_back(symbol);

            return CID;
        }

        void setSocket(const Socket_t client) { this->cli_socket = client; }
        void setHeader(const SockIn_t client) { this->cli_header = client; }
        // void setClientID(const std::vector<char>& CID) { this->cid = CID;  }

        bool connectClient();
        bool disconnectClient();

        bool isConnected();

        bool sendClientData(void *, uint32_t);
        bool sendClientData(const std::vector<char>&);
        bool sendClientData(const std::string&);
    };
}

#endif // SRV_SOCKET