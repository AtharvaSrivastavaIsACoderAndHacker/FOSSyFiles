#include"oneInclude.h"

using namespace std;

 
int main(int argc, char const *argv[]){

    windowsWSAInitNClean(0); // init

    // Threads
    std::ofstream multicastLog("./textFiles/multicastLogMain.txt", std::ios::app);
    std::thread multicastReceiverThread([&]() {
        auto* oldBuf = std::cout.rdbuf(multicastLog.rdbuf());
        multicastReceiveDevicesDetailsInLAN();
        std::cout.rdbuf(oldBuf);
    });

    std::thread multicastBroadcasterThread([&]() {
        auto* oldBuf = std::cout.rdbuf(multicastLog.rdbuf());
        multicastTheIdentityInLAN("mainThreadName", 12000);
        std::cout.rdbuf(oldBuf);
    });
    std::thread listener(listenFinal, KEYS.publicKey, 12000);
    // std::string instanceName; cin>>instanceName;
    // int port = 12000;
    






    // call gui

    while (true) {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }


    // listenFinal(KEYS.publicKey, 12000);

    // std::string path = "./sample/minecraft.png";
    // fragmentEncryptAndSendAFile(path, CLIENT.clientSocket, CLIENTFULL, calculate_file_sha256(path));    







    if(1 == 0){
        // stopAndCloseAll = true;
        // multicastReceiverThread.join();
    }
    windowsWSAInitNClean(0); // clear
    std::cout<<"---------------------------------End Of Main thread Run ---- Log ends here at " + getCurrentTime()<<endl;

    return 0;
}



