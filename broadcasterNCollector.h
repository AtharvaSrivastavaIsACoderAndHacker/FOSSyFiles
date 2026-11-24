#define brodIncluded

#include <string>
#include <cstring>
#ifndef sharedStructsIncluded
    #include "sharedStructs.h"
#endif

using namespace std;
extern std::vector<DiscoveredDeviceInfo> discoveredDevices; // defined in oneInclude.h
extern mutex broadcasterMtx;
extern condition_variable broadcasterCV;
extern std::atomic<bool> haltReceiving;
extern std::atomic<bool> haltSending;
extern std::atomic<bool> stopAndCloseAll;


std::string get_local_lan_ip() {

    windowsWSAInitNClean(0); // init
    socket_t sock = socket(AF_INET, SOCK_DGRAM, 0);

    if (sock == INVALID_SOCKET){
        windowsWSAInitNClean(1); // clear
        return "";
    }
    
    sockaddr_in remote_addr{};
    remote_addr.sin_family = AF_INET;
    remote_addr.sin_port = htons(443);
    inet_pton(AF_INET, "8.8.8.8", &remote_addr.sin_addr);


    if (connect(sock, reinterpret_cast<sockaddr*>(&remote_addr), sizeof(remote_addr)) < 0) {
        windowsWSAInitNClean(1);
        closesocket(sock); // its platform independent, i've defined macros to replace it if it were POSIX
        return "";
    }


    sockaddr_in local_addr{};
    socklen_t addr_len = sizeof(local_addr);
    if (getsockname(sock, reinterpret_cast<sockaddr*>(&local_addr), &addr_len) < 0) {
        windowsWSAInitNClean(1);
        closesocket(sock);
        return "";
    }

    char ip_buffer[INET_ADDRSTRLEN] = {0};
    inet_ntop(AF_INET, &local_addr.sin_addr, ip_buffer, sizeof(ip_buffer));


    closesocket(sock);
    windowsWSAInitNClean(1);


    return std::string(ip_buffer);
}

bool multicastTheIdentityInLAN(std::string name, int port){

    std::string selfIP = get_local_lan_ip();
    MulticastedInfo multicastPacket{};
    strcpy(multicastPacket.ip, selfIP.c_str());
    strcpy(multicastPacket.name, name.c_str());
    multicastPacket.port = port;
    strcpy(multicastPacket.magic, "_____multicastInfoDatagram_____fossyfiles_____");

    if (selfIP == ""){
        std::cerr << "[broadcasterNCollector.h][multicastTheIdentityInLAN()][" + getCurrentTime() + "] Failed to find any usable UDP IP Interfaces.\n";
        return 1;
    }

    windowsWSAInitNClean(0);


    // SETUP SOCKET
    socket_t multicastSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (multicastSocket == INVALID_SOCKET) {
        std::cerr << "[broadcasterNCollector.h][multicastTheIdentityInLAN()][" + getCurrentTime() + "] Failed to create Multicast Broadcaster Socket.\n";
        return 1;
    }

    
    // SETUP SOCKET OPTIONS
    unsigned char ttl = 255; // TTL for multicast routing -- 1 == local subnet only; 255 == reaches 
    unsigned char loop = 1; // 1 ==> allow the multicast to loopback to the same device(this), this allows multipe instances
                            // will change this to 0 later but right now its needed for TESTING AND DEBUGGING  on the same device loopback
    setsockopt(multicastSocket, IPPROTO_IP, IP_MULTICAST_TTL, reinterpret_cast<const char*>(&ttl), sizeof(ttl));
    setsockopt(multicastSocket, IPPROTO_IP, IP_MULTICAST_LOOP, reinterpret_cast<const char*>(&loop), sizeof(loop));
    cout<<"[broadcasterNCollector.h][multicastTheIdentityInLAN()][" + getCurrentTime() + "] Warning: multicast loopback to same device is enabled (set to 1) for beta testing purposes -- turn it off for production "<<endl;



    // SETUP THE MULTICAST DESTINATION/ADDRESS
    sockaddr_in multicastAddr{};
    multicastAddr.sin_family = AF_INET;
    multicastAddr.sin_port = htons(MULTICAST_PORT);
    inet_pton(AF_INET, MULTICAST_IP.c_str(), &multicastAddr.sin_addr);


    // SEND MULTICAST INFO DATAGRAM -- this can be updated to be a loop and the function might be called as a parallel unblocked thread to keep sending ttl packets -- unfinished
    while(!stopAndCloseAll){

        int bytesSent = sendto(multicastSocket, reinterpret_cast<char *>(&multicastPacket), static_cast<int>(sizeof(multicastPacket)), 0,
        reinterpret_cast<sockaddr*>(&multicastAddr), sizeof(multicastAddr));
        if (bytesSent == SOCKET_ERROR) {
            std::cerr << "[broadcasterNCollector.h][multicastTheIdentityInLAN()][" + getCurrentTime() + "] sendto failed.\n";
        }
        else {
            // std::cout << "[broadcasterNCollector.h][multicastTheIdentityInLAN()] Sent " << bytesSent << " bytes to " << MULTICAST_IP << ":" << MULTICAST_PORT << "\n";
        }


        {
            unique_lock<mutex> halt(broadcasterMtx);
            broadcasterCV.wait(halt, [](){ return !haltSending; });
        }
        unique_lock<mutex> lock(broadcasterMtx);
        broadcasterCV.wait_for(lock, 1s, [] {return haltSending || stopAndCloseAll;});
        if(stopAndCloseAll) break;
        

    }



    // STOP AND CLEANUP
    closesocket(multicastSocket);
    windowsWSAInitNClean(1);
    return 0;
}

bool multicastReceiveDevicesDetailsInLAN(){

    windowsWSAInitNClean(0);

    // SETUP SOCKET
    socket_t multicastSocket = socket(AF_INET, SOCK_DGRAM, IPPROTO_UDP);
    if (multicastSocket == INVALID_SOCKET) {
        std::cerr << "[broadcasterNCollector.h][multicastReceiveDevicesDetailsInLAN()][" + getCurrentTime() + "] Failed to create Multicast Socket.\n";
        return 1;
    }

    // set options to make it reusable and not hog it to just itself. this allows multiple isntances fo the program to subscribe to this multicast
    int reuse = 1;
    if (setsockopt(multicastSocket, SOL_SOCKET, SO_REUSEADDR, reinterpret_cast<const char*>(&reuse), sizeof(reuse)) == SOCKET_ERROR) {
        std::cerr << "[broadcasterNCollector.h][multicastReceiveDevicesDetailsInLAN()][" + getCurrentTime() + "] setsockopt(SO_REUSEADDR) failed.\n";
        closesocket(multicastSocket);
        return 1;
    }

    // BIND TO INADDR_ANY AND THE MULTICAST_PORT
    sockaddr_in localAddr{};
    localAddr.sin_family = AF_INET;
    localAddr.sin_port = htons(MULTICAST_PORT);
    localAddr.sin_addr.s_addr = htonl(INADDR_ANY);

    if (bind(multicastSocket, reinterpret_cast<sockaddr*>(&localAddr), sizeof(localAddr)) == SOCKET_ERROR) {
        std::cerr << "[broadcasterNCollector.h][multicastReceiveDevicesDetailsInLAN()][" + getCurrentTime() + "] Bind failed.\n";
        closesocket(multicastSocket);
        return 1;
    }

    // SUBSCRIBE TO THE MULTICAST

    ip_mreq mreq{};
    inet_pton(AF_INET, MULTICAST_IP.c_str(), &mreq.imr_multiaddr);
    mreq.imr_interface.s_addr = htonl(INADDR_ANY);
    if (setsockopt(multicastSocket, IPPROTO_IP, IP_ADD_MEMBERSHIP, reinterpret_cast<const char*>(&mreq), sizeof(mreq)) == SOCKET_ERROR) {
        std::cerr << "[broadcasterNCollector.h][multicastReceiveDevicesDetailsInLAN()][" + getCurrentTime() + "] setsockopt(IP_ADD_MEMBERSHIP) failed.\n";
        closesocket(multicastSocket);
        return 1;
    }

    std::cout << "[broadcasterNCollector.h][multicastReceiveDevicesDetailsInLAN()][" + getCurrentTime() + "] Subscribed to FOSSyFiles Multicast " << MULTICAST_IP << ":" << MULTICAST_PORT << " !\n";



    // DSCOVERY RECEIVER LOOP -- will be a loop of receiving and the function will be run as a parallel thread to keep updating the array of structured defined outside via mtx and atomic locks

    char buffer[1024];
    sockaddr_in senderAddr{}; // to receive the sender detaisl from where the udp datagram came
    socklen_t senderAddrLen = sizeof(senderAddr);

    while (!stopAndCloseAll){

        int bytesReceived = recvfrom(multicastSocket, buffer, sizeof(buffer) - 1, 0,
                                     reinterpret_cast<sockaddr*>(&senderAddr), &senderAddrLen);
        if (bytesReceived == SOCKET_ERROR) {
            std::cerr << "recvfrom failed.\n";
            break;
        }
        buffer[bytesReceived] = '\0';
        char senderIp[INET_ADDRSTRLEN];
        inet_ntop(AF_INET, &senderAddr.sin_addr, senderIp, INET_ADDRSTRLEN);
        if (bytesReceived == sizeof(MulticastedInfo)){
            MulticastedInfo* receivedDeviceInfo = reinterpret_cast<MulticastedInfo*>(&buffer);
            if (strncmp(receivedDeviceInfo->magic,"_____multicastInfoDatagram_____fossyfiles_____",128) == 0){
                DiscoveredDeviceInfo newDevice{};
                newDevice.ip = receivedDeviceInfo->ip;
                newDevice.name = receivedDeviceInfo->name;
                newDevice.port = receivedDeviceInfo->port;
                
                {
                    lock_guard<mutex> lock(broadcasterMtx);
                    for(const auto& i : discoveredDevices){
                        if(i.ip == newDevice.ip){
                            senderAddrLen = sizeof(senderAddr);
                            goto ignoreThisDuplicateEntry; // ignore if duplicate !
                        }
                    }
                
                    if(0==1){
                        ignoreThisDuplicateEntry:
                        continue;
                    }

                    // update discovery list
                    discoveredDevices.push_back(newDevice);
                    // std::cout<<newDevice.ip<<newDevice.name<<std::endl;
                }
                
            }

            else continue;
        }
        

        {
            unique_lock<mutex> halt(broadcasterMtx);
            broadcasterCV.wait(halt, [](){ return !haltReceiving; });
        }
        if(stopAndCloseAll) break;

        senderAddrLen = sizeof(senderAddr);
    }





    // UNSUBSCRIBE AND CLOSE
    setsockopt(multicastSocket, IPPROTO_IP, IP_DROP_MEMBERSHIP, reinterpret_cast<const char*>(&mreq), sizeof(mreq));
    std::cout << "[broadcasterNCollector.h][multicastReceiveDevicesDetailsInLAN()][" + getCurrentTime() + "] Unsubscribed from FOSSyFiles Multicast " << MULTICAST_IP << ":" << MULTICAST_PORT << " !\n";
    closesocket(multicastSocket);
    windowsWSAInitNClean(1);
    return 0;
}
