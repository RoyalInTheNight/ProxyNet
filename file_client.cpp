#include <arpa/inet.h>
#include <iostream>
#include <fstream>
#include <ostream>
#include <string>
#include <sys/socket.h>
#include <netinet/in.h>
#include <unistd.h>
#include <vector>

#define ESTABILISH_BYTE 0xfe

int main(int argc, char **argv) {
    if (argc < 3) {
        std::cout << "Usage: " << argv[0] << " <ip-address> <port>" << std::endl;

        return -1;
    }

    // Создаем сокет
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    if (serverSocket < 0) {
        std::cout << "Ошибка создания сокета" << std::endl;
        return 1;
    }

    // Настраиваем адрес и порт
    struct sockaddr_in serverAddress{};
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(std::stoi(argv[2])); // Используйте свой порт
    serverAddress.sin_addr.s_addr = inet_addr(argv[1]);

    std::cout << argv[1] << std::endl;
    std::cout << argv[2] << std::endl;

    // Привязываем сокет к адресу и порту
    if (connect(serverSocket, (struct sockaddr *)&serverAddress, sizeof(serverAddress)) < 0) {
        std::cout << "Ошибка привязки сокета" << std::endl;
        return 1;
    }

    std::vector<char> esta;

    esta.push_back(ESTABILISH_BYTE);

    if (::send(serverSocket, esta.data(), esta.size(), 0) < 0) {
        std::cout << "estabilish connection error" << std::endl;

        return -3;
    }

    std::vector<char> filename(1024);

    if (recv(serverSocket, filename.data(), 1024, 0) <= 0) {
        std::cout << "Ошибка получения имени файла" << std::endl;

        return -1;
    }

    // Создаем файл для записи полученных данных
    std::ofstream outputFile(filename.data(), std::ofstream::binary);
    if (!outputFile.is_open()) {
        std::cout << "Ошибка создания файла для записи" << std::endl;
        return 1;
    }

    // Читаем данные от клиента и записываем в файл
    char buffer[1024];
    int bytesRead;
    while ((bytesRead = recv(serverSocket, buffer, sizeof(buffer), 0)) > 0) {
        outputFile.write(buffer, bytesRead);
    }

    // Закрываем соединение и файл
    close(serverSocket);
    outputFile.close();

    std::cout << "Файл успешно принят и сохранен." << std::endl;

    return 0;
}