#ifndef __SOCKET
#define __SOCKET

#ifdef _WIN32
#  include <ws2tcpip.h>
#  include <winsock2.h>
#else
#  include <sys/socket.h>
#  include <sys/un.h>
#  include <netinet/in.h>
#  include <arpa/inet.h>
#endif // _WIN32

#include <vector>
#include <string>

#include <cstdint>
#include <cstdlib>

#include "../logging/logging.h"
#include "../crypto/sha.h"

#define INT16 0x7fff

enum LSAData {
    Socket = -0xff,
    SocketBind,
    SocketListen,
    SocketAccept,
    SocketSend,
    SocketRead,
    SocketSendFile,
    SocketReadFile,
    SocketSOData,
    SocketEstabilish
};

namespace ProxyNet {
    const uint32_t listen_queue = 1024;

    enum class ThreadStatus : uint8_t {
        threadEnable = 0xa1,
        threadDisable,
    };

    enum class SSL : uint8_t {
        SSL_Enable = 0xb1,
        SSL_Disable
    };

    #ifdef _WIN32
        typedef SOCKET      socket_t;
        typedef sockaddr_in header_t;
        typedef int32_t     headln_t;
    #else
        typedef int32_t     socket_t;
        typedef sockaddr_in header_t;
        typedef socklen_t   headln_t;
    #endif // _WIN32

    struct SocketData {
        socket_t     socket;
        header_t     header;
        headln_t headerSize;

    public:
        SocketData(const uint32_t address, const uint16_t port, int32_t socket_, const int32_t af_family) {
            header.sin_addr.s_addr = address;
            header.sin_port = port;
            header.sin_family = af_family;
            headerSize = sizeof header;

            socket = socket_;
        }

        SocketData(int32_t socket_, const int32_t af_family) {
            socket            = socket_;
            header.sin_family = af_family;
        }

        SocketData(const SocketData& sockData) {
            socket     = sockData.socket;
            header     = sockData.header;
            headerSize = sockData.headerSize;
        }
    };

    struct ESData {
        uint32_t  address;
        uint16_t     port;

        int32_t af_family;
        uint8_t      flag;

        uint32_t SSL_PKEY;
    } __attribute__((packed));

    class Socket
         : public logging::Logging {
    private:
        class Client : logging::Logging {
        private:
            SocketData *sodata;

            std::string    CID;
            uint32_t       KEY;
            SSL            ssl;

            uint8_t       flag;

        public:
            Client(SocketData *, const std::string&, const SSL = SSL::SSL_Disable);
            Client(const Client&);

            [[nodiscard]] SocketData *getSOData() const;
            [[nodiscard]] std::string    getCID() const;

            void setSOData(const uint32_t, const uint16_t, const uint32_t, const uint8_t);

            int32_t send(const std::string&);
            int32_t send(const std::vector<char>&);
            int32_t send(const void *, const uint32_t);

            int32_t read(std::string&);
            int32_t read(std::vector<char>&);
            int32_t read(void *, const uint32_t);

            int32_t sendFile(const std::string&);
            int32_t sendFile(const std::vector<char>&);
            int32_t sendFile(const void *, const uint32_t);

            int32_t readFile(const std::string&);
            int32_t readFile(const std::vector<char>&);
            int32_t readFile(const void *, const uint32_t);

            void disconnect();

            ~Client();
        };

        SSL                  ssl;
        ThreadStatus      thread;
        SocketData *      sodata;
        std::vector<Client> list;

        uint32_t clientCount = 0;

    public:
        typedef std::string            str_t;
        typedef std::vector<char>      vec_t;
        typedef void *                void_t;
        typedef SocketData *           sda_t;
        typedef ThreadStatus           thr_t;
        typedef logging::LoggingStatus lst_t;
        typedef std::vector<Client>    cli_t;

        Socket(sda_t, const str_t& = ".Socket", const lst_t& = lst_t::loggingDisable);
        Socket(const Socket&, const str_t& = ".Socket", const lst_t& = lst_t::loggingDisable);
        Socket(const str_t& = ".Socket", const lst_t& = lst_t::loggingDisable);

        [[nodiscard]] sda_t    getSocketData() const;
        [[nodiscard]] thr_t  getThreadStatus() const;
        [[nodiscard]] lst_t getLoggingStatus() const;
        [[nodiscard]] cli_t    getClientList() const;

        void setSocketData(sda_t);
        void setThreadStatus(const thr_t&);
        void setLoggingStatus(const lst_t&);
        void setLoggingPath(const str_t&);

        int32_t socketInit();
        int32_t socketAccept();
        
        int32_t sendTo(const std::string&, const str_t&);
        int32_t sendTo(const std::string&, const vec_t&);
        int32_t sendTo(const std::string&, const void *, const uint32_t);
        int32_t sendAll(const str_t&);
        int32_t sendAll(const vec_t&);
        int32_t sendAll(const void *, const uint32_t);

        int32_t readTo(const std::string&, str_t&);
        int32_t readTo(const std::string&, vec_t&);
        int32_t readTo(const std::string&, void *, const uint32_t);

        int32_t sendFileTo(const std::string&, const str_t&);
        int32_t sendFileTo(const std::string&, const vec_t&);
        int32_t sendFileTo(const std::string&, const void_t, const uint32_t);
        int32_t sendFileAll(const str_t&);
        int32_t sendFileAll(const vec_t&);
        int32_t sendFileAll(const void_t, const uint32_t);

        int32_t readFileTo(const std::string&, const str_t&);
        int32_t readFileTo(const std::string&, const vec_t&);
        int32_t readFileTo(const std::string&, const void_t, const uint32_t);

        void disconnectTo(const std::string&);
        void disconnectAll();

        ~Socket();
    };
}

#endif // __SOCKET