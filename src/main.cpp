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


using namespace std;


GlobalState gs;
GlobalConfig gc;

#include "rest_api.h"

std::atomic<bool> running(true);


void signalHandler(int signal) {
    std::cout << "\nCaught signal " << signal << ". Ending call..." << std::endl;
    running = false;
    gs.notify_exit();
}


int main(int argc, char* argv[]){
    using namespace std;

     // Set up signal handler for Ctrl+C (SIGINT)
    signal(SIGINT, signalHandler);
    vector<Pipeline> pipelines;

    if(argc > 1){
        gc.load(argv[1]);
    }else{
        std::cerr << "Please provide path to config file as the first argument!"<<std::endl;
        return 1;
    }

    json const & config_pipelines = gc.get_pipelines_config();
    // Iterate over the array of pipelines
    for(auto it = config_pipelines.begin(); it != config_pipelines.end(); ++it){
        pipelines.emplace_back(it.key(), *it);
    }

    for(auto & pipeline: pipelines){
        pipeline.start();
    }

    CivetServer* server = start_server();
    if (!server) {
        return 1;
    }  // FIXME: Notify threads

    while(running){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    for(auto & pipeline: pipelines){
        pipeline.stop();
    }

    stop_server(server);

    std::cout << "end of pipeline"<<std::endl;

    return 0;
}
