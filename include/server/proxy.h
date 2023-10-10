#ifndef PROXY_SERVER
#define PROXY_SERVER

#include "../socket/server_socket.h"
#include "../crypto/sha256.h"
#include "../crypto/rsa.h"

class ProxyServer
    : public sys::SocketServer {
private:
public:
};

#endif // PROXY_SERVER