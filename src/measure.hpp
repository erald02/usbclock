#pragma once
#include <string>
#include <CLI/CLI.hpp>

void setup_measure_subcommands(CLI::App& app);
void measure_latency(const std::string& device_path, int duration);
void measure_consistency(const std::string& device_path, int duration);
