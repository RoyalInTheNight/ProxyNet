#ifdef _WIN32
#include <WinSock2.h>
#define NIX(exp)
#define WIN(exp) exp
#else
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#define NIX(exp) exp
#define WIN(exp)
#endif

#include <iostream>
#include <string>
#include <vector>

#define ESTABILISH_BYTE 0xfe
#define SHELL_BYTE      0xfd

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cout << "Usage" << argv[0] << " <ip-address> <port>" << std::endl;

        return -1;
    }

#ifdef _WIN32
    WSADATA wsa;

    WSAStartup(MAKEWORD(2, 2), &wsa);
#endif

    unsigned short port = std::stoi(argv[2]);

    sockaddr_in addr;

    addr.sin_addr.WIN(S_un.S_addr)NIX(s_addr) = inet_addr(argv[1]);
    addr.sin_port             = htons(port);
    addr.sin_family           = AF_INET;

    WIN(SOCKET)NIX(int) sock = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    connect(sock, (sockaddr *)&addr, sizeof(addr));

    std::vector<char> buffer(40000);

    buffer.at(0) = ESTABILISH_BYTE;

    ::send(sock, buffer.data(), buffer.size(), 0);

    char shell = SHELL_BYTE;

    while (::recv(sock, buffer.data(), 40000, 0) > 0) {
        if (buffer.at(0) == shell) {
            std::string shell_buffer;

            for (auto& _shell : buffer)
                if (_shell != shell)
                    shell_buffer.push_back(_shell);

            std::cout << "command read: " << shell_buffer << std::endl;
        }

        std::cout << buffer.data() << std::endl;

        for (auto& b : buffer) b = '\0';
    }

    return 0;
}