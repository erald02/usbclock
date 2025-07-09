#pragma once
#include <libusb-1.0/libusb.h>
#include <stdexcept>
#include <CLI/CLI.hpp>

class UsbContext {
public:
    UsbContext() {
        if (libusb_init(&ctx) < 0) {
            throw std::runtime_error("Failed to initialize libusb");
        }
    }

    ~UsbContext() {
        if (ctx) libusb_exit(ctx);
    }

    libusb_context* get() const { return ctx; }

private:
    libusb_context *ctx = nullptr;
};

void setup_devices_subcommands(CLI::App& app);
void list_devices();
