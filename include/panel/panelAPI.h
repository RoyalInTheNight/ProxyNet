#ifndef PANEL_API
#define PANEL_API

#include <functional>
#include <vector>
#ifndef _WIN32
#  include <sys/un.h>
#endif // _WIN32

#include "../server/server.h"

class panel_API {
private:
    typedef sockaddr_un        hPanel_t;
    typedef int32_t            sPanel_t;
    typedef std::vector<char> pBuffer_t;
    typedef void                 void_t;
    typedef std::function<std::string()> sHandler_t;

private:
    hPanel_t panel_unix;
    sPanel_t panel_sock;
    sPanel_t panel_accept;

    pBuffer_t panel_tx;
    pBuffer_t panel_rx;

    std::string unixPath;

    sHandler_t server_handler;

public:
    panel_API();
    panel_API(hPanel_t, pBuffer_t *, pBuffer_t *);
    panel_API(hPanel_t, sPanel_t);
    panel_API(hPanel_t);

    [[nodiscard]] std::string GetUnixPath()   const;
    [[nodiscard]] hPanel_t    GetUnix()       const;
    [[nodiscard]] sPanel_t    GetUnixSock()   const;
    [[nodiscard]] sPanel_t    GetUnixAccept() const;
    [[nodiscard]] pBuffer_t   GetUnixTX()     const;
    [[nodiscard]] pBuffer_t   GetUnixRX()     const;

    void_t SetUnix(const hPanel_t&);
    void_t SetUnixSock(const sPanel_t);
    void_t SetUnixAccept(const sPanel_t);
    void_t SetUnixTX(pBuffer_t *);
    void_t SetUnixRX(pBuffer_t *);
    void_t SetUnixPath(const std::string&);
    void_t SetPanelHandler();

    bool    UnixCreate();
    bool    UnixListen();
    bool UnixAPI_Start();
    bool  UnixDestruct();

    ~panel_API();
};

#endif // PANEL_API