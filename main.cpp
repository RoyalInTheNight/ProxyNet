#include <iostream>
#include "include/socket/server_socket.h"

int main() {
    sys::SocketServer server = 4445;

    server.setThreadType(sys::ThreadMode::single_thread);
    server.setType(sys::SocketType::tcp_socket);

    if (!server.socketInit())
        std::cout << "[FAILED]Server init failed" << std::endl;

    std::thread([&]() -> void {
        std::cout << "[ INFO ]Start listening" << std::endl;

        while (server.socketListenConnection());
    }).detach();

    while (true) {
        if (server.getClients().size() > 2) {
            std::cout << "Client list: " << std::endl;

            for (auto& clList : server.getClients())
                std::cout << "ip - " << clList.host << " port - " << clList.port << std::endl;

            server.getClients().clear();
        }
    }
}