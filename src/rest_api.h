#ifndef __M2E_BRIDGE_REST_API_H__
#define __M2E_BRIDGE_REST_API_H__

#include "CivetServer.h"

CivetServer* start_server();

void stop_server(CivetServer* server);

#endif 