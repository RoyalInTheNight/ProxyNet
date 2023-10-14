#include <iostream>
#include "include/socket/server_socket.h"
#include "include/shell/shell.h"
#include "include/crypto/sha256.h"

int32_t main(int32_t argc, char **argv) {
    if (argc < 2) {
        std::cout << "Usage: " << argv[0] << " <port>" << std::endl;

        return -1;
    }

    std::vector<std::string> data;
    sys::SocketServer      server;

    sha256 sha_crypto;

    uint16_t port = std::stoi(argv[1]);

    server.setPort(port);
    server.setThreadType(sys::ThreadMode::single_thread);
    server.setType(sys::SocketType::tcp_socket);

    if (!server.socketInit()) {
        std::cout << "[FAILED]Socket init error" << std::endl;

        return -1;
    }

    else
        std::cout << "[  OK  ]Socket init success" << std::endl;

    std::thread([&]() -> void {
        while (server.socketListenConnection())
            data.push_back(sha_crypto.hash(std::to_string(server.getClients().at(server.getClients().size() - 1).port), sha256::sha256_options::option_string_hash));

        exit(0);
    }).detach();
    
    std::string sh;

    while (true) {
        std::cout << "command> ";
        std::cin >> sh;

        if (sh == "clients") {
            std::vector<sockaddr_in>             clientOutput(server.getClients().size());
            std::vector<sys::ClientConnectionData> clientList = server.getClients();

            int count = 0;

            if (clientList.size() <= 0)
                std::cout << "list client empty" << std::endl;

            for (uint32_t i = 0; i < clientList.size(); i++) {
                clientOutput.at(i) WIN(.sin_addr.S_un.S_addr)LINUX(.sin_addr.s_addr) = clientList.at(i).host;
                clientOutput.at(i).sin_port = htons(clientList.at(i).port);
                clientOutput.at(i).sin_family = AF_INET;
            }

            for (auto& client : clientOutput) {
                std::cout << "-------------------"                       << std::endl;
                std::cout << "client_id: " << data.at(count)             << std::endl;
                std::cout << "address:   " << inet_ntoa(client.sin_addr) << std::endl;
                std::cout << "port:      " << htons(client.sin_port)     << std::endl;
                std::cout << "___________________"                       << std::endl;

                ++count;
            }
        }

        if (sh == "thread_mode") {
            if (server.getThreadMode() == sys::ThreadMode::single_thread)
                std::cout << "signle thread mode" << std::endl;

            else
                std::cout << "multi thread mode" << std::endl;
        }

        if (sh == "type") {
            if (server.getSocketType() == sys::SocketType::tcp_socket)
                std::cout << "server transport protocol - tcp" << std::endl;

            else
                std::cout << "server transport protocol - udp" << std::endl;
        }

        if (sh == "send") {
            int i = 0;

            std::string client_data;

            for (const auto& clientId : data) {
                std::cout << "____________________"           << std::endl
                          << i + 1 << " " << clientId.c_str() << std::endl;

                ++i;
            }

            std::cout << "enter_client_id> ";
            std::cin  >> i;
            std::cout << "client_data> ";
            std::cin >> client_data;

            server.connectBy(server.getClients().at(i - 1).host, server.getClients().at(i - 1).port);
            server.sendBy(server.getClients().at(i - 1).host, server.getClients().at(i - 1).port, client_data);
            server.disconnectBy(server.getClients().at(i - 1).host, server.getClients().at(i - 1).port);
        }

        if (sh == "check_clients")
            std::cout << "active clients: " << server.checkBot() << std::endl;

        if (sh == "exit")
            exit(0);
    }

    std::cout << "[ INFO ]Shutdown socket" << std::endl;

    server.disconnectAll();
}