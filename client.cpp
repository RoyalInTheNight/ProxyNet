#include <WinSock2.h>
#include <iostream>
#include <string>
#include <vector>

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cout << "Usage" << argv[0] << " <ip-address> <port>" << std::endl;

        return -1;
    }

    WSADATA wsa;

    WSAStartup(MAKEWORD(2, 2), &wsa);

    unsigned short port = std::stoi(argv[2]);

    sockaddr_in addr;

    addr.sin_addr.S_un.S_addr = inet_addr(argv[1]);
    addr.sin_port             = htons(port);
    addr.sin_family           = AF_INET;

    SOCKET sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    connect(sock, (sockaddr *)&addr, sizeof(addr));

    std::vector<char> buffer(40000);

    while (::recv(sock, buffer.data(), 40000, 0) > 0) {
        std::cout << buffer.data() << std::endl;
    }

    return 0;
}