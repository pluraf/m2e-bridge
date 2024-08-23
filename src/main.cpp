#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <atomic>
#include <csignal>

#include "pipeline.h"

std::atomic<bool> running(true);

void signalHandler(int signal) {
    std::cout << "\nCaught signal " << signal << ". Ending call..." << std::endl;
    running = false;
}

int main(int argc, char* argv[]) {


     // Set up signal handler for Ctrl+C (SIGINT)
    std::signal(SIGINT, signalHandler);


    std::string file_path = PROJECT_SOURCE_DIR + std::string("/pipeline_test.json");

    std::ifstream file(file_path);
    if (!file) {
        std::cerr << "Failed to open file: " << file_path << std::endl;
        return 1;
    }

    std::stringstream buffer;
    buffer << file.rdbuf();

    file.close();

    std::string json_str = buffer.str();
    Pipeline myPipeline(json_str);
    myPipeline.start();
    std::cout<<"started pipeline"<<std::endl;
    while(running){
        std::this_thread::sleep_for(std::chrono::seconds(1));

    }
    myPipeline.stop();
    std::cout << "end of pipeline"<<std::endl;
    return 0;
}