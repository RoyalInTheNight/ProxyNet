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

enum LSAData {
    Socket = -0xff,
    SocketBind,
    SocketListen,
    SocketAccept,
    SocketSend,
    SocketRead,
    SocketSendFile,
    SocketReadFile,
    SocketSOData
};

namespace ProxyNet {
    enum class ThreadStatus : uint8_t {
        threadEnable = 0xa1,
        threadDisable,
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

        SocketData(const SocketData& sockData) {
            socket     = sockData.socket;
            header     = sockData.header;
            headerSize = sockData.headerSize;
        }
    };

    class Socket
         : public logging::Logging {
    private:
        class Client {
        private:
            SocketData *sodata;
            uint32_t       CID;

        public:
            Client(SocketData *, const uint32_t);
            Client(const Client&);

            virtual int32_t send(const std::string&);
            virtual int32_t send(const std::vector<char>&);
            virtual int32_t send(const void *, const uint32_t);

            virtual int32_t read(const std::string&);
            virtual int32_t read(const std::vector<char>&);
            virtual int32_t read(const void *, const uint32_t);

            virtual int32_t sendFile(const std::string&);
            virtual int32_t sendFile(const std::vector<char>&);
            virtual int32_t sendFile(const void *, const uint32_t);

            virtual int32_t readFile(const std::string&);
            virtual int32_t readFile(const std::vector<char>&);
            virtual int32_t readFile(const void *, const uint32_t);

            void disconnect();

            ~Client();
        };

        ThreadStatus      thread;
        SocketData *      sodata;
        std::vector<Client> list;

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
        
        int32_t sendTo(const uint32_t, const str_t&);
        int32_t sendTo(const uint32_t, const vec_t&);
        int32_t sendTo(const uint32_t, const void_t, const uint32_t);
        int32_t sendAll(const str_t&);
        int32_t sendAll(const vec_t&);
        int32_t sendAll(const void_t, const uint32_t);

        int32_t readTo(const uint32_t, str_t&);
        int32_t readTo(const uint32_t, vec_t&);
        int32_t readTo(const uint32_t, void_t, const uint32_t);

        int32_t sendFileTo(const uint32_t, const str_t&);
        int32_t sendFileTo(const uint32_t, const vec_t&);
        int32_t sendFileTo(const uint32_t, const void_t, const uint32_t);
        int32_t sendFileAll(const str_t&);
        int32_t sendFileAll(const vec_t&);
        int32_t sendFileAll(const void_t, const uint32_t);

        int32_t readFileTo(const uint32_t, const str_t&);
        int32_t readFileTo(const uint32_t, const vec_t&);
        int32_t readFileTo(const uint32_t, const void_t, const uint32_t);

        void disconnectTo(const uint32_t);
        void disconnectAll();

        ~Socket();
    };
}

#endif // __SOCKET