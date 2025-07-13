#pragma once
#include <CLI/CLI.hpp>

void setup_measure_subcommands(CLI::App& app);
void measure_latency(int duration, bool verbose);
std::string select_device_interactively();
