#include <CLI/CLI.hpp>
#include "measure.hpp"
#include "overclock.hpp"
#include "devices.hpp"

int main(int argc, char** argv) {
    CLI::App app{"usbclock - USB overclocking and input latency"};

    setup_measure_subcommands(app);
    setup_overclock_subcommand(app);
    setup_devices_subcommands(app);

    CLI11_PARSE(app, argc, argv);
    return 0;
}
