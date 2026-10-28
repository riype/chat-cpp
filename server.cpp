#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
#include <vector>
#include <mutex>
using namespace std;

vector<int> clients;
mutex clientMutex;

void handClient(int clientSocket){
char buffer[1024];
while (true){
	int bytesReceived = recv(clientSocket, buffer, 1024, 0);
        if (bytesReceived <= 0){
                break;
                }

	buffer[bytesReceived] = '\0';
	cout<<"Получено: "<<buffer<<endl;
{
	lock_guard<mutex> lock(clientMutex);
	for (int i = 0; i < clients.size(); i++){
		send(clients[i], buffer, bytesReceived, 0);
		}
}
	}
close(clientSocket);
cout<<"Клиент отключился"<<endl;

{
	lock_guard<mutex> lock(clientMutex);
        for (int i = 0; i < clients.size(); i++) {
		if (clients[i] == clientSocket) {
	                clients.erase(clients.begin() + i);
	                break;
    }
}

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
cout<<"Сервер привязан к порту 8080"<<endl;

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
{
	lock_guard<mutex> lock(clientMutex);
	clients.push_back(clientSocket);
}

    thread t(handClient, clientSocket);
    t.detach();
}

close(serverSocket);

	return 0;
}
