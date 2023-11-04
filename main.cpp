#include <iostream>
#include "include/socket/server_socket.h"
#include "include/crypto/sha256.h"

int32_t main(int32_t argc, char **argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;

        return -1;
    }

    uint16_t port = std::stoi(argv[1]);

    sys::SocketServer server(port);

    server.setThreadType(sys::ThreadMode::single_thread);
    server.setType(sys::SocketType::tcp_socket);

    server.socketInit();

    std::thread([&]() -> void {
        while (server.socketListenConnection());

        std::cout << "Server shutdown, accept error" << std::endl;
        exit(0);
    }).detach();

    std::string sh;

    while (true) {
        std::cout << "command> ";
        std::cin >> sh;

        if (sh == "getClients") {
            std::vector<std::string> CID                   = server.getCID();
            std::vector<sys::ClientConnectionData> clients = server.getClients();

            if (CID.size() != clients.size())
                std::cout << "List broken" << std::endl;

            else {
                for (int32_t i = 0; i < CID.size(); i++)
                    std::cout << "_______________"                       << std::endl
                              << "ClientId: "      << CID.at(i).data()   << std::endl
                              << "ClientAddress: " << clients.at(i).host << std::endl;
            }
        }

        if (sh == "getCID")
            for (const auto& CID : server.getCID())
                std::cout << "_______________" << std::endl
                          << CID.data()        << std::endl;

        if (sh == "send") {
            uint32_t CID = 0;

            int count = 0;

            for (const auto& CID_id : server.getCID()) {
                std::cout << count << ". " << CID_id.data() << std::endl;

                ++count;
            }

            std::cout << "CID> ";
            std::cin >> CID;

            if (CID >= server.getCID().size() || CID < 0)
                std::cout << "Invalid CID" << std::endl;

            else {
                std::string data;

                std::cout << "data> ";
                std::cin >> data;

                if (!server.sendBy(server.getCID().at(CID), data))
                    std::cout << "Send error" << std::endl;

                else
                    std::cout << "Message delivered" << std::endl;
            }
        }

        if (sh == "sendAll") {
            std::string data;

            std::cout << "data> ";
            std::cin >> data;

            if (!server.sendAll(data))
                std::cout << "Send error" << std::endl;

            else
                std::cout << "Message delivered" << std::endl;
        }
    }
}