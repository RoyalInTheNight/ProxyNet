#include "include/logging/logging.h"
#include "include/socket/socket.h"
#include <arpa/inet.h>
#include <netinet/in.h>
#include <string>
#include <sys/socket.h>

int32_t main(int32_t argc, char **argv) {
    if (argc < 2) {
        std::cout << argv[0] << " <port>" << std::endl;
        return -0x1;
    }

    ProxyNet::Socket server(
        new ProxyNet::SocketData(
            htonl(
                inet_addr(
                    "0.0.0.0"
                    )
                ), 
                
                htons(
                    std::stoi(
                        argv[
                            1
                        ]
                    )
                ), 
                
                ::socket(
                    AF_INET, 
                    SOCK_STREAM, 
                    IPPROTO_TCP
                ), 
                
                AF_INET
        ),

        ".server_log",
        logging::LoggingStatus::loggingStdout
    );

    server.setThreadStatus(ProxyNet::ThreadStatus::threadDisable);

    if (!server.socketInit())
        return -0x2;

    if (server.socketAccept() >= 0) {
        std::cout << "socket accept called" << std::endl;

        return -0x2;
    }

    return 0;
}