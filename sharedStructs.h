#ifndef SHARED_TYPES_H
#define SHARED_TYPES_H

#include <cstdint>
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

#pragma pack(push, 1)
struct KnockPacket {
    char magic[128];
    int tcpReturn;
};
#pragma pack(pop)

struct Connection {
    sockaddr_in peerAddr;
    int peerSocket;
};

#endif
