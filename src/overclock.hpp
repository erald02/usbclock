#pragma once
#include <string>
#include <CLI/CLI.hpp>

void setup_overclock_subcommand(CLI::App& app);
void set_polling_rate(const std::string& device, int rate);
