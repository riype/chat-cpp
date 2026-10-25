#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
using namespace std;

int main() {

int clientSocket = socket (AF_INET, SOCK_STREAM, 0);
if (clientSocket == -1) {
	perror("socket");
	return 1;
}
sockaddr_in serverAddr;
memset(&serverAddr, 0, sizeof(serverAddr));
serverAddr.sin_family = AF_INET;
serverAddr.sin_addr.s_addr = inet_addr("127.0.0.1");
serverAddr.sin_port = htons(8080);

if (connect(clientSocket, (sockaddr*)&serverAddr, sizeof(serverAddr)) == -1) {
        perror("connect");
        close(clientSocket);
        return 1;
}
cout<<"Подключено к серверу"<<endl;

char buffer[1024];

cout<<"Введите сообщение: ";
cin.getline(buffer,1024);

if (send(clientSocket, buffer, strlen(buffer), 0) == -1){
	perror("send");
}

memset(buffer,0,1024);
int bytes = recv(clientSocket, buffer, 1024, 0);

if (bytes>0){
	cout<<"Ответ сервера: "<<buffer<<endl;
}
close(clientSocket);

	return 0;
}
