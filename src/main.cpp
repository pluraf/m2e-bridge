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


using namespace std;


GlobalState gs;

std::atomic<bool> running(true);


void signalHandler(int signal) {
    std::cout << "\nCaught signal " << signal << ". Ending call..." << std::endl;
    running = false;
    gs.notify_exit();
}


int main(int argc, char* argv[]){
     // Set up signal handler for Ctrl+C (SIGINT)
    signal(SIGINT, signalHandler);
    vector<Pipeline> pipelines;

    std::string config_path {std::format("{}/{}", PROJECT_SOURCE_DIR, "pipeline_test.json")};

    if(argc == 2){
        config_path = std::string(argv[1]);
    }

    std::ifstream file(config_path);
    if (!file) {
        std::cerr << "Failed to open file: " << config_path << std::endl;
        return 1;
    }
    std::stringstream buffer;
    buffer << file.rdbuf();
    file.close();

    std::string json_str = buffer.str();
    json parsed = json::parse(json_str);

    json config_pipelines = parsed["pipelines"];

    // Iterate over the array of pipelines
    for(auto it = config_pipelines.begin(); it != config_pipelines.end(); ++it){
        pipelines.emplace_back(it.key(), *it);
    }

    for(auto & pipeline: pipelines){
        pipeline.start();
    }

    while(running){
        std::this_thread::sleep_for(std::chrono::seconds(1));
    }

    for(auto & pipeline: pipelines){
        pipeline.stop();
    }

    std::cout << "end of pipeline"<<std::endl;
    return 0;
}