#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>

#define SHELL_MODE_BYTE  0xfd
#define UPDATE_MODE_BYTE 0xfc
#define ESTABILISH_BYTE  0xfe

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <ip-address> <port>" << std::endl;

        return -1;
    }

    const char shell_mode      = SHELL_MODE_BYTE;
    const char update_mode     = UPDATE_MODE_BYTE;
    const char estabilish_mode = ESTABILISH_BYTE;

    const std::string ip_address = argv[1];
    const std::string port       = argv[2];

    sockaddr_in imx8mm;

    imx8mm.sin_addr.s_addr = inet_addr(ip_address.c_str());
    imx8mm.sin_port        = htons(std::stoi(port));
    imx8mm.sin_family      = AF_INET;

    int32_t imx8mm_socket = ::socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);

    if (imx8mm_socket < 0) {
        std::cout << "[FAILED]Socket create failed" << std::endl;

        return -2;
    }

    int32_t imx8mm_debug = ::connect(imx8mm_socket, (const sockaddr *)&imx8mm, sizeof(imx8mm));

    if (imx8mm_debug < 0) {
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

    while (::recv(imx8mm_socket, imx8mm_comm_buffer.data(), imx8mm_comm_buffer.size(), 0)) {
        if (imx8mm_comm_buffer.size() > 0) {
            for (auto& com : imx8mm_comm_buffer) {
                if (com == update_mode)
                    update_mode_byte = true;

                if (com == shell_mode)
                    shell_mode_byte = true;
            }

            if (update_mode_byte) {
                std::string path = imx8mm_comm_buffer.data();

                /*for (int32_t i = 0; i < path.size(); i++)
                    if (path.at(i) == update_mode)
                        path.at(i) = 0;*/

                path.pop_back();

                std::cout << "[ INFO ]Readed file name: " << path << std::endl << "[ INFO ]File name size: " << path.size() << std::endl;
                
                std::ofstream outputFile(path, std::ofstream::binary);

                if (!outputFile.is_open()) {
                    std::cout << "[FAILED]File create error" << std::endl;

                    close(imx8mm_socket);
                    outputFile.close();

                    return -3;
                }

                char fileBuffer[1024];
                int32_t bytesRead = 0;

                while ((bytesRead = ::recv(imx8mm_socket, fileBuffer, sizeof(fileBuffer), 0)) > 0)
                    outputFile.write(fileBuffer, bytesRead);

                outputFile.close();

                update_mode_byte = false;

                std::cout << "[ INFO ]File get success" << std::endl;
                
                imx8mm_comm_buffer.clear();
                imx8mm_comm_buffer.resize(imx8mm_comm_buffer.size());
            }

            else if (shell_mode_byte) {
                shell_mode_byte = false;
            }

            else
                std::cout << "mode not found: " << imx8mm_comm_buffer.data() << std::endl;
        }
    }
}