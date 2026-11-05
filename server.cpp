#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
#include <map>
#include <string>
using namespace std;

map<int, string> clients;
mutex clientMutex;

void handClient(int clientSocket){
    char buffer[1024];
    int nameBytes = recv(clientSocket, buffer, 1024, 0);
    if (nameBytes <= 0) {
        close(clientSocket);
        return;
    }
    
    // Обрезаем возможный \n на конце имени
    string senderName(buffer, nameBytes);
    if (!senderName.empty() && senderName.back() == '\n') {
        senderName.pop_back();
    }

    {
        lock_guard<mutex> lock(clientMutex);
        clients[clientSocket] = senderName;
    }

    string currentMessage; // Буфер для накопления данных

    while (true){
        int bytesReceived = recv(clientSocket, buffer, 1024, 0);
        if (bytesReceived <= 0){
            break;
        }

        buffer[bytesReceived] = '\0';
        currentMessage += buffer; // Накапливаем пришедшие данные

        // Ищем полные сообщения, разделенные \n
        size_t pos;
        while ((pos = currentMessage.find('\n')) != string::npos) {
            string fullMsg = currentMessage.substr(0, pos);
            currentMessage.erase(0, pos + 1); // Удаляем обработанное

            cout << "Получено: " << fullMsg << endl;
            
            {
                lock_guard<mutex> lock(clientMutex);
                for (auto const& [sock, name] : clients){
                    // Используем имя отправителя (senderName), а не получателя
                    string outMsg = "[" + senderName + "]: " + fullMsg + "\n";
                    send(sock, outMsg.c_str(), outMsg.length(), 0);
                }
            }
        }
    }
    
    close(clientSocket);
    cout << "Клиент " << senderName << " отключился" << endl;

    {
        lock_guard<mutex> lock(clientMutex);
        clients.erase(clientSocket);
    }
}

int main() {
    int serverSocket = socket (AF_INET, SOCK_STREAM, 0);
    if (serverSocket == -1) {
        perror("socket");
        return 1;
    }
    sockaddr_in serverAddr;
    memset(&serverAddr, 0, sizeof(serverAddr));
    serverAddr.sin_family = AF_INET;
    serverAddr.sin_addr.s_addr = INADDR_ANY;
    serverAddr.sin_port = htons(8080);

    if (bind(serverSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("bind");
        close(serverSocket);
        return 1;
    }
    cout << "Сервер привязан к порту 8080" << endl;

    if (listen(serverSocket, 5) == -1) {
        perror("listen");
        close(serverSocket);
        return 1;
    }
    cout << "Сервер слушает порт 8080" << endl;

    while (true) {
        int clientSocket = accept(serverSocket, nullptr, nullptr);
        if (clientSocket == -1) {
            perror("accept");
            continue;
        }
        cout << "Новый клиент подключился" << endl;

        thread t(handClient, clientSocket);
        t.detach();
    }

    close(serverSocket);
    return 0;
}
