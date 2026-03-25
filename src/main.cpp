/* SPDX-License-Identifier: MIT */

/*
Copyright (c) 2024-2026 Pluraf Embedded AB <code@pluraf.com>

Permission is hereby granted, free of charge, to any person obtaining a copy of
this software and associated documentation files (the “Software”), to deal in
the Software without restriction, including without limitation the rights to
use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies
of the Software, and to permit persons to whom the Software is furnished to
do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all
copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED “AS IS”, WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL
THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM,
OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
IN THE SOFTWARE.
*/


#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <atomic>
#include <csignal>
#include <fmt/core.h>
#include <curl/curl.h>

#include "m2e_aliases.h"
#include "global_state.h"
#include "global_config.h"
#include "pipeline.h"
#include "pipeline_supervisor.h"
#include "shared_objects.h"

#ifdef WITH_ZEROMQ_API
    #include "zmq_listner.h"
#endif

#ifdef WITH_HTTP_API
    #include "rest_api.h"
#endif

#ifdef WITH_HTTP_LISTENER
    #include "listeners/http_listener.h"
#endif

using namespace std;

extern GlobalConfig gc;
extern GlobalState gs;

std::atomic<bool> g_running(true);

void signalHandler(int signal) {
    std::cout << "\nCaught signal " << signal << ". Ending call..." << std::endl;
    g_running = false;
}

int main(int argc, char* argv[])
{
    using namespace std;

     // Set up signal handler for Ctrl+C (SIGINT)
    signal(SIGINT, signalHandler);

    if( argc > 1 )
    {
        gc.load(argv[1]);
    }
    else
    {
        std::cerr << "Please provide path to config file as the first argument!"<<std::endl;
        return 1;
    }

    SharedObjects::get_instance().init(gc.get_shared_objects_config());

#ifdef _WITH_CURL
    if( curl_global_init(CURL_GLOBAL_DEFAULT) != 0 )
    {
        std::cerr << "Failed to initialize libcurl!\n";
        return 1;
    }
#endif

    PipelineSupervisor *ps = PipelineSupervisor::get_instance();
    ps->start_all();

#ifdef WITH_ZEROMQ_API
    ZmqListener *zmq = ZmqListener::get_instance();
    zmq->start();
#endif

#ifdef WITH_HTTP_API
    CivetServer * server = start_server();
#endif

#ifdef WITH_HTTP_LISTENER
    HTTPListener::start();
#endif

    while( g_running )
    {
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    gs.notify_exit();

    ps->terminate_all();

#ifdef _WITH_CURL
    curl_global_cleanup();
#endif

#ifdef WITH_HTTP_API
    stop_server(server);
#endif

#ifdef WITH_ZEROMQ_API
    zmq->stop();
#endif

    return 0;
}
