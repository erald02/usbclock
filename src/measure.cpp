#include "measure.hpp"
#include <iostream>

void setup_measure_subcommands(CLI::App& app) {
    auto measure = app.add_subcommand("measure", "Measurement utilities");

    std::string device_path;
    int duration = 5;

    auto latency = measure->add_subcommand("latency", "Measure input latency");
    latency->add_option("-p,--path", device_path, "Device path")->required();
    latency->add_option("-t,--time", duration, "Duration in seconds");
    latency->callback([&]() { measure_latency(device_path, duration); });

    auto consistency = measure->add_subcommand("consistency", "Measure polling consistency");
    consistency->add_option("-p,--path", device_path, "Device path")->required();
    consistency->add_option("-t,--time", duration, "Duration in seconds");
    consistency->callback([&]() { measure_consistency(device_path, duration); });
}

// Implement measure_latency and measure_consistency in detail here
void measure_latency(const std::string& device_path, int duration) {
    std::cout << "Measuring latency for " << device_path << " for " << duration << " seconds" << std::endl;
    
}

void measure_consistency(const std::string& device_path, int duration) {
    std::cout << "Measuring consistency for " << device_path << " for " << duration << " seconds" << std::endl;
}
