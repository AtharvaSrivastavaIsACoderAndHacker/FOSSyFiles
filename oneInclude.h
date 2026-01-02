#include <atomic>
#include "connectionInitiator.h"
#include "listen.h"
 
#ifndef metricsHeaderIncluded
#include "metrics.h"
#endif
#ifndef keyGenHeaderIncluded
#include "RSA_keygen.h"
#endif
#ifndef sharedStructsIncluded
#include "sharedStructs.h"
#endif

DHKeyPair KEYS = generateDHKeyPair();
