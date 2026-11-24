#include"oneInclude.h"
extern atomic<bool> stopFlag;

using namespace std;


int main(int argc, char const *argv[]){

    windowsWSAInitNClean(0); // init

    // Threads
    std::ofstream multicastLog("./textFiles/multicastLogClient.txt", std::ios::app);
    std::thread multicastReceiverThread([&]() {
        auto* oldBuf = std::cout.rdbuf(multicastLog.rdbuf());
        multicastReceiveDevicesDetailsInLAN();
        std::cout.rdbuf(oldBuf);
    });

    std::thread multicastBroadcasterThread([&]() {
        auto* oldBuf = std::cout.rdbuf(multicastLog.rdbuf());
        multicastTheIdentityInLAN("clientThreadName", 12000);
        std::cout.rdbuf(oldBuf);
    });
    




    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    // while(true){
    //     for (int i = 0; i < discoveredDevices.size(); i++){
    //         cout<<discoveredDevices[i].name<<endl;
    //     }
    //     cout<<discoveredDevices.size()<<endl;
    // }
    
    

    string server_ip;
    cin>>server_ip;
    int port = 12000;
    
    connectTo(server_ip, port, 12001, KEYS.publicKey, 12002);

    cout<<"Server Info --> "<<inet_ntoa(peerWhoReceived.peerAddr.sin_addr)<<":"<<ntohs(peerWhoReceived.peerAddr.sin_port)<<endl;


    // auto start = std::chrono::steady_clock::now();
    // bool success = defragmentDecryptAndReceiveAFile(peerWhoReceived.peerSocket,peerWhoReceived,12002, std::string("Received")+slash);
    // auto end = std::chrono::steady_clock::now();
    // auto duration = std::chrono::duration_cast<std::chrono::milliseconds>(end - start);
    // if(success){
    //     std::ofstream out("textFiles/logs.txt", std::ios::app);
    //     std::string log = receivedFile.fileName+" "+std::to_string(receivedFile.fileSizeInBytes)+" "+std::to_string(receivedFile.chunkSizeInBytes)+" "+std::to_string(receivedFile.totalChunks)+" "+std::to_string(duration.count())+"  \n";
    //     out.write(log.data(), log.size());
    // }    






    if(1 == 0){
        stopAndCloseAll = true;
        multicastReceiverThread.join();
    }

    windowsWSAInitNClean(1); // clear
    std::cout<<"---------------------------------End Of Client thread Run ---- Log ends here at " + getCurrentTime()<<endl;
    
    return 0;
}


