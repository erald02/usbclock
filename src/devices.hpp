#pragma once
#include <libusb-1.0/libusb.h>
#include <stdexcept>
#include <CLI/CLI.hpp>
#include <string>

struct UsbDeviceInfo {
    int index;
    uint16_t vendor_id;
    uint16_t product_id;
    std::string manufacturer;
    std::string product;
    bool could_open;
};


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
void print_devices(const std::vector<UsbDeviceInfo>& devices);
std::vector<UsbDeviceInfo> fetch_devices();
