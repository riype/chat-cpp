#include <iostream>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <cstring>
#include <thread>
using namespace std;

void recMessage(int clientSocket){
    string currentMessage;
    while (true){
        char buf[1024];
        memset(buf,0,1024);
        int bytes = recv(clientSocket, buf, 1024, 0);
        if (bytes <= 0){
            break;
        }

        buf[bytes] = '\0';
	currentMessage += buf;
	size_t pos;
        while ((pos = currentMessage.find('\n')) != string::npos) {
            string fullMsg = currentMessage.substr(0, pos);
            currentMessage.erase(0, pos + 1);

            cout << "\r" << fullMsg << "\nВведите сообщение: " << flush;
        }
    }
}

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

char name[50];
cout<<"Введите ваше имя: ";
cin.getline(name, 50);

string nameMsg = string(name) + "\n";
send(clientSocket, nameMsg.c_str(), nameMsg.length(), 0);

thread t(recMessage, clientSocket);

char buffer[1024];

while(true){
	cout<<"Введите сообщение: "<<flush;
	cin.getline(buffer,1024);

	if(strcmp(buffer, "exit") == 0){
		break;
}
string msg = string(buffer) + "\n";
if (send(clientSocket, msg.c_str(), msg.length(), 0) == -1){
		perror("send");
		break;
}

}
shutdown(clientSocket,SHUT_RDWR);
t.join();
close(clientSocket);

	return 0;
}
