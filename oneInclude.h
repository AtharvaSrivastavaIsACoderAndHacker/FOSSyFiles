std::vector<DiscoveredDeviceInfo> discoveredDevices = {};
mutex broadcasterMtx;
condition_variable broadcasterCV;
atomic<bool> haltReceiving = false;
atomic<bool> haltSending = false;
atomic<bool> stopAndCloseAll = false;


#include <atomic>
#include <vector>
#include "connectionInitiator.h"
#include "listen.h"
 
#ifndef metricsHeaderIncluded
#include "metrics.h"atomic<bool>
#endif
#ifndef keyGenHeaderIncluded
#include "RSA_keygen.h"
#endif
#ifndef sharedStructsIncluded
#include "sharedStructs.h"
#endif
#ifndef guiIncluded
#include "gui.h"
#endif
#ifndef brodIncluded
#include "broadcasterNCollector.h"
#endif

inline FileInfo receivedFile;

DHKeyPair KEYS = generateDHKeyPair();

