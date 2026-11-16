#include <atomic>
#include <vector>
#include <mutex>
#include <condition_variable>

#ifndef sharedStructsIncluded
#include "sharedStructs.h"
#endif

std::vector<DiscoveredDeviceInfo> discoveredDevices = {};
std::mutex broadcasterMtx;
std::condition_variable broadcasterCV;
std::atomic<bool> haltReceiving = false;
std::atomic<bool> haltSending = false;
std::atomic<bool> stopAndCloseAll = false;

#include "connectionInitiator.h"
#include "listen.h"
 
#ifndef metricsHeaderIncluded
#include "metrics.h"atomic<bool>
#endif
#ifndef keyGenHeaderIncluded
#include "RSA_keygen.h"
#endif
#ifndef guiIncluded
#include "gui.h"
#endif
#ifndef brodIncluded
#include "broadcasterNCollector.h"
#endif

inline FileInfo receivedFile;

DHKeyPair KEYS = generateDHKeyPair();

