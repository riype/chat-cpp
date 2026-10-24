#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
using namespace std;

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
cout<<"Сервер привязан к порту 8080"<<endl;

if (listen(serverSocket, 5) == -1) {
    perror("listen");
    close(serverSocket);
    return 1;
}
cout << "Сервер слушает порт 8080. Ждёт подключения" << endl;

int clientSocket = accept(serverSocket, nullptr, nullptr);
if (clientSocket == -1) {
    perror("accept");
    close(serverSocket);
    return 1;
}
cout << "Клиент подключился" << endl;

char buffer[1024];
while (true) {
    int bytesReceived = recv(clientSocket, buffer, sizeof(buffer) - 1, 0);

    if (bytesReceived <= 0) {
        cout << "Клиент отключился" << endl;
        break;
    }

    buffer[bytesReceived] = '\0';
    cout << "Получено: " << buffer << endl;

    send(clientSocket, buffer, bytesReceived, 0);
}

close(clientSocket);
close(serverSocket);

	return 0;
}
