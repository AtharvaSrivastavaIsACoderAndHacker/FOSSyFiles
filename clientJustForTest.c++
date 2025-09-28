#include<iostream>
#include <winsock2.h>
#include <ws2tcpip.h>

using namespace std;

int main(int argc, char const *argv[]){

    
    WSADATA wsaData;
    if (WSAStartup(MAKEWORD(2,2), &wsaData) != 0) {
        cerr << "WSAStartup failed\n";
        return 1;
    }

    string server_ip;
    cin>>server_ip;
    int port = 12000;

    int clientSocket = socket(AF_INET, SOCK_STREAM, 0);
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    if (inet_pton(AF_INET, server_ip.c_str(), &serverAddress.sin_addr) <= 0) {
        cerr << "Invalid server IP address: " << server_ip << "\n";
    }
    serverAddress.sin_port = htons(port);

    int connectedOrNot = connect(clientSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    cout<<"connected ? --> "<<connectedOrNot<<endl;
    while(1){
        string message; cin>>message;
        int bytesSent = send(clientSocket, message.c_str(), strlen(message.c_str()), 0);
        if (bytesSent == SOCKET_ERROR) {
            cerr << "Send failed: " << WSAGetLastError() << "\n";
        } else {
            cout << "Sent to server: " << message << "\n";
        }
    }
    WSACleanup();
    
    return 0;
}