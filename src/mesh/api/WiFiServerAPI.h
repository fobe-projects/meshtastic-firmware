#pragma once

#include "ServerAPI.h"
#include <WiFi.h>

#if HAS_ETHERNET && defined(USE_WS5500)
#include <ETHClass2.h>
#define ETH ETH2
#endif // HAS_ETHERNET

/**
 * Provides both debug printing and, if the client starts sending protobufs to us, switches to send/receive protobufs
 * (and starts dropping debug printing - FIXME, eventually those prints should be encapsulated in protobufs).
 */
class WiFiServerAPI : public ServerAPI<WiFiClient>
{
  public:
    explicit WiFiServerAPI(WiFiClient &_client);
};

/**
 * Listens for incoming connections and does accepts and creates instances of WiFiServerAPI as needed
 */
class WiFiServerPort : public APIServerPort<WiFiServerAPI, WiFiServer>
{
  public:
    explicit WiFiServerPort(int port);
};

void initApiServer(int port = SERVER_API_DEFAULT_PORT);
void deInitApiServer();