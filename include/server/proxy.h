#ifndef PROXY_SERVER
#define PROXY_SERVER

#include "../socket/server_socket.h"
#include "../crypto/sha256.h"
#include "../crypto/rsa.h"

#include <map>

class ProxyServer
    : public sys::SocketServer {
private:
    sha256 sha_crypto;

    typedef struct {
        std::string clientId;
        std::string clientAddress;
        std::string clientPort;
    } clientServiceData;

    std::vector<clientServiceData> clientMap;

public:
    ProxyServer();
    ProxyServer(const ProxyServer&);
    ProxyServer(const sys::SocketServer&);
    ProxyServer(const sys::SocketType&, const sys::ThreadMode&);

    bool startProxyServer();
    bool connectWithProxy(const uint32_t proxy_host,
                          const uint16_t proxy_port,
                          const uint32_t dst_host,
                          const uint16_t dst_port);

    bool connectWithoutProxy(const uint32_t dst_host,
                             const uint16_t dst_port);
 
    bool sendWithProxy(const uint32_t proxy_client, 
                       const uint16_t proxy_port, 
                       const uint32_t dst_host,     
                       const uint16_t dst_port, 
                       const std::string& data);

    bool sendWithoutProxy(const uint32_t dst_host, 
                          const uint16_t dst_port, 
                          const std::string& data);

    ~ProxyServer();
};

#endif // PROXY_SERVER