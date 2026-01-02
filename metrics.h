// code in this file is not entirely mine, ive obviously copied it because i don't rote learn libraries and their functions
// if u expect any coder to write code entirely without yanking, (and you code ENTIRELY on you own) either you are god her/himself manifested and i respect you, or you are just stupid and incompetent who yanks stuff but yells at me for doing the same !

#pragma once

#include <cstdint>
#include <string>
#include <chrono>
#include <thread>
#include <iostream>

#ifdef _WIN32
    #include <windows.h>
    #include <winsock2.h>
    #include <ws2tcpip.h>
    #pragma comment(lib, "Ws2_32.lib")
#else
    #include <sys/sysinfo.h>
    #include <fstream>
    #include <sys/types.h>
    #include <sys/socket.h>
    #include <arpa/inet.h>
    #include <unistd.h>
#endif

#define metricsHeaderIncluded

    
inline uint64_t getAvailableRAM() {
    #ifdef _WIN32
        MEMORYSTATUSEX memInfo;
        memInfo.dwLength = sizeof(MEMORYSTATUSEX);
        GlobalMemoryStatusEx(&memInfo);
        return memInfo.ullAvailPhys; 
    #else
        struct sysinfo info;
        sysinfo(&info);
        return info.freeram;
    #endif
}


#ifdef _WIN32
    inline double getCPUUsage() {
        static FILETIME prevIdleTime, prevKernelTime, prevUserTime;
        FILETIME idleTime, kernelTime, userTime;

        if (!GetSystemTimes(&idleTime, &kernelTime, &userTime))
            return -1.0;

        uint64_t idle = (uint64_t(idleTime.dwHighDateTime) << 32) | idleTime.dwLowDateTime;
        uint64_t kernel = (uint64_t(kernelTime.dwHighDateTime) << 32) | kernelTime.dwLowDateTime;
        uint64_t user = (uint64_t(userTime.dwHighDateTime) << 32) | userTime.dwLowDateTime;

        uint64_t prevIdle = (uint64_t(prevIdleTime.dwHighDateTime) << 32) | prevIdleTime.dwLowDateTime;
        uint64_t prevKernel = (uint64_t(prevKernelTime.dwHighDateTime) << 32) | prevKernelTime.dwLowDateTime;
        uint64_t prevUser = (uint64_t(prevUserTime.dwHighDateTime) << 32) | prevUserTime.dwLowDateTime;

        uint64_t idleDiff = idle - prevIdle;
        uint64_t totalDiff = (kernel - prevKernel) + (user - prevUser);

        prevIdleTime = idleTime;
        prevKernelTime = kernelTime;
        prevUserTime = userTime;

        return (totalDiff == 0) ? 0.0 : (double)(totalDiff - idleDiff) * 100.0 / totalDiff;
    }
#else
    inline double getCPUUsage() {
        std::ifstream stat("/proc/stat");
        std::string line;
        std::getline(stat, line);
        stat.close();

        uint64_t user, nice, system, idle, iowait, irq, softirq, steal;
        sscanf(line.c_str(), "cpu  %llu %llu %llu %llu %llu %llu %llu %llu",
               &user, &nice, &system, &idle, &iowait, &irq, &softirq, &steal);

        static uint64_t prevIdle = 0, prevTotal = 0;
        uint64_t idleTime = idle + iowait;
        uint64_t totalTime = user + nice + system + idle + iowait + irq + softirq + steal;

        uint64_t diffIdle = idleTime - prevIdle;
        uint64_t diffTotal = totalTime - prevTotal;

        prevIdle = idleTime;
        prevTotal = totalTime;

        if (diffTotal == 0) return 0.0;
        return (double)(diffTotal - diffIdle) * 100.0 / diffTotal;
    }
#endif



size_t calculateChunkSize(size_t fileSize, int rttMs) {

    // tried to get this right but doe to some OS constraints, this doesn't always work ! :(
    uint64_t freeRAM = getAvailableRAM();
    double cpuUsage = getCPUUsage();

    size_t baseChunk = std::min(fileSize, freeRAM / 50);

    baseChunk = static_cast<size_t>(baseChunk * (1.0 - cpuUsage / 100.0));

    // latency based adjustment 
    if(rttMs > 100) baseChunk = std::min(baseChunk, static_cast<size_t>(10000));   // bad latency == small chunks
    else if(rttMs > 50) baseChunk = std::min(baseChunk, static_cast<size_t>(500000)); // moderate latency
    else if(rttMs < 5) baseChunk = std::min(baseChunk * 4, static_cast<size_t>(10485760)); // best latency ---> 10 MB
    else if(rttMs < 40) baseChunk = std::min(baseChunk * 2, static_cast<size_t>(10485760)); // good latency → up to 10 MB

    // limits
    if(baseChunk < 10000) baseChunk = 10000;           // min 10 KB
    if(baseChunk > 100000) baseChunk = 100000;     // max 100 KB

    return baseChunk;
}
