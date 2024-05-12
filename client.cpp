#include "include/socket/client_socket.h"
#include <string>
#include <vector>

#include <iostream>

int main() {
    SocketClient client;

    std::vector<ClientTypes::ServerConnectionData> srvConnData;

    client.setConnectionData("127.0.0.1", 4445);
    client.setThreadType(ClientTypes::ThreadStatus::mthread_status);

    srvConnData = client.getServerConnectionData();

    for (auto& _: srvConnData) {
        client.socketInit(_.CID);
        client.socketConnect(_.CID);
    }

    while (ClientTypes::SocketStatus::err_socket_ok == client.recvHandler())
        std::cout << "ClientTypes::SocketStatus::err_socket_ok" << std::endl;

    return 0;
}