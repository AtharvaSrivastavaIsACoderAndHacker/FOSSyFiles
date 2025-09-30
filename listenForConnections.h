#include<iostream>
#include <utility>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include "sharedStructs.h"

#ifdef _WIN32
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <netinet/in.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

using namespace std;

struct ConnectionRequest {
    sockaddr_in clientAddr;
    int clientSocket;
    EVP_PKEY* publicKey;
};
struct ServerInfo {
    int serverSocket;
    sockaddr_in serverAddress;
};

extern atomic<bool> stopListening;
sockaddr_in connectToInitiator;
// Shared state
extern mutex mtx;
extern condition_variable cv;
extern bool newRequest;
extern bool stopTHIS;
extern bool decisionReady;
extern bool acceptConnection;
extern bool connected;
extern bool asking;
extern ConnectionRequest pendingRequest;
extern ServerInfo server;
extern string clientIPViaUdp;
extern int clientPortViaUdp;

ConnectionRequest CLIENT;

inline void listenAndAccept(string server_ip, int port){

    
    int sockfdUdp; 
    char buffer[1024]; 
    struct sockaddr_in servaddr, cliaddr;  
    if ( (sockfdUdp = socket(AF_INET, SOCK_DGRAM, 0)) < 0 ) { 
        perror("socket creation failed"); 
        exit(EXIT_FAILURE); 
    } 
    memset(&servaddr, 0, sizeof(servaddr)); 
    memset(&cliaddr, 0, sizeof(cliaddr)); 
    servaddr.sin_family    = AF_INET;  
    servaddr.sin_addr.s_addr = inet_addr(server_ip.c_str()); 
    servaddr.sin_port = htons(port);  
    if ( bind(sockfdUdp, (const struct sockaddr *)&servaddr,  
            sizeof(servaddr)) < 0 ) 
    { 
        perror("bind failed"); 
        exit(EXIT_FAILURE); 
    } 
    char bufferUdp[1024];
    struct sockaddr_in clientAddrUdp;
    socklen_t clientAddrLen = sizeof(clientAddrUdp);





    int sockfd, newsock;
    int serverSocket = socket(AF_INET, SOCK_STREAM, 0);
    sockaddr_in serverAddress;
    serverAddress.sin_family = AF_INET;
    serverAddress.sin_port = htons(port);
    if (inet_pton(AF_INET, server_ip.c_str(), &serverAddress.sin_addr) <= 0) {
        cerr << "Invalid server IP address: " << server_ip << "\n";
    }

    int bindingReturn = bind(serverSocket, (struct sockaddr*)&serverAddress, sizeof(serverAddress));
    if (bindingReturn == SOCKET_ERROR) perror("error in binding !");
    int eCodeOnListeningForConnection = listen(serverSocket, 1);
    if(eCodeOnListeningForConnection!=0) perror("error in listening !");
    cout<<bindingReturn<<" and "<<eCodeOnListeningForConnection<<endl;
    







    while (!stopListening.load()) {
        #ifdef _WIN32
        u_long mode = 1;
        ioctlsocket(sockfdUdp, FIONBIO, &mode); // Non-blocking
        #else
        int flags = fcntl(sockfdUdp, F_GETFL, 0);
        fcntl(sockfdUdp, F_SETFL, flags | O_NONBLOCK);
        #endif

        int bytesReceivedForIncomingConnections = recvfrom(sockfdUdp, buffer, 1024, 0, (struct sockaddr*)&clientAddrUdp, &clientAddrLen);
        if (bytesReceivedForIncomingConnections > 0){
                if (bytesReceivedForIncomingConnections < sizeof(KnockPacket)) {
                    cout << "Received incomplete packet, ignoring\n";
                    continue;
                }
                KnockPacket* pkt = reinterpret_cast<KnockPacket*>(buffer);
                // cout<<"Received a Valid FossyDatagram"<<endl;
                // cout<<pkt->magic<<endl;
                // cout<<pkt->tcpReturn<<endl;

                // if FOSSyFiles sent this :
                if (strncmp(pkt->magic, "_____connectionRequestDatagram_____fossyfiles_____", 128) == 0) {
                    {   
                        
                        
                        unique_lock<mutex> lock(mtx);
                        clientIPViaUdp = inet_ntoa(clientAddrUdp.sin_addr);
                        clientPortViaUdp = pkt->tcpReturn;
                        cout<<clientIPViaUdp<<" and "<<clientPortViaUdp<<endl;
                        newRequest = true;
                        CLIENT.publicKey = pkt->publicKey;

                    }
                    cout<<buffer<<endl;
                } 
        }
        else{
            continue;

        }
        

        if (newRequest){
            // cout<<"log from listener header, 1 request came from a client !"<<inet_ntoa(clientAddrUdp.sin_addr)<<endl;

                // ask main thread to respond to this
                {   
                    unique_lock<mutex> lock(mtx);
                    pendingRequest.clientAddr = clientAddrUdp;
                    newRequest = true;
                    asking = true;
                    // cout<<"locked by listener"<<endl;
                    cout.flush();
                }
                cv.notify_one(); // notify main thread
                cout<<"Listener : Waiting for Main --> "<<acceptConnection<<endl;
                // Wait for main thread's decision
                {   
                    unique_lock<mutex> lock(mtx);
                    cv.wait(lock, [](){ return decisionReady; });
                    
                    if (acceptConnection) {
                        int ServerSocket = socket(AF_INET, SOCK_STREAM, 0);
                        // clientIPViaUdp = inet_ntoa(clientAddrUdp.sin_addr);
                        // clientPortViaUdp = pkt->tcpReturn;
                        cout<<clientPortViaUdp<<endl;
                        cout<<clientIPViaUdp<<endl;
                        
                        connectToInitiator.sin_family = AF_INET;
                        connectToInitiator.sin_addr.s_addr = inet_addr(clientIPViaUdp.c_str());
                        connectToInitiator.sin_port = htons(clientPortViaUdp);
                        int connectedOrNot = connect(ServerSocket, (struct sockaddr*)&connectToInitiator, sizeof(connectToInitiator));
                        cout << "Connection accepted from "
                             << inet_ntoa(connectToInitiator.sin_addr) << " : " 
                             << ntohs(connectToInitiator.sin_port) << "\n";
                             cout.flush();
                             connected = true;
                             acceptConnection = false;
                             newRequest = false;
                             asking = false;
                             CLIENT.clientAddr = connectToInitiator;
                             CLIENT.clientSocket = ServerSocket;
                    } 
                    else {
                        cout << "Connection rejected.\n";
                        newRequest = false;
                        decisionReady = false;
                        acceptConnection = false;
                        connected = false;
                        asking = false;
                    }
                }
            
        }




    server.serverAddress = serverAddress;
    server.serverSocket = serverSocket;
    // end of while loop
}


}