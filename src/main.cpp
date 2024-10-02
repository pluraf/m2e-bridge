#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <atomic>
#include <csignal>
#include <format>

#include "m2e_aliases.h"
#include "pipeline.h"
#include "global_state.h"
#include "global_config.h"
#include "pipeline_supervisor.h"


using namespace std;

GlobalConfig gc;
GlobalState gs;

#include "rest_api.h"

std::atomic<bool> g_running(true);


void signalHandler(int signal) {
    std::cout << "\nCaught signal " << signal << ". Ending call..." << std::endl;
    g_running = false;
}


int main(int argc, char* argv[]){
    using namespace std;

     // Set up signal handler for Ctrl+C (SIGINT)
    signal(SIGINT, signalHandler);

    if(argc > 1){
        gc.load(argv[1]);
    }else{
        std::cerr << "Please provide path to config file as the first argument!"<<std::endl;
        return 1;
    }
    PipelineSupervisor *ps = gs.get_pipeline_supervisor();
    ps->start();

    CivetServer* server = start_server();
    if(! server) g_running = false;

    while(g_running){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }
    gs.notify_exit();

    ps->stop();

    stop_server(server);

    return 0;
}
