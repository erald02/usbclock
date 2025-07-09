#include "overclock.hpp"
#include <iostream>

void setup_overclock_subcommand(CLI::App& app) {
    std::string device;
    int rate;

    auto overclock = app.add_subcommand("overclock", "Set USB polling rate");
    overclock->add_option("-d,--device", device, "Device identifier")->required();
    overclock->add_option("-r,--rate", rate, "Polling rate (ms)")->required();
    overclock->callback([&]() { set_polling_rate(device, rate); });
}

void set_polling_rate(const std::string& device, int rate) {
    // Implement polling rate adjustment logic here
    // For mice: echo options to /sys/module/usbhid/parameters/mousepoll
    // For controllers: print warning if unsupported
}
