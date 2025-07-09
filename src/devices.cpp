#include "devices.hpp"
#include <iostream>
#include <filesystem>
#include <libusb-1.0/libusb.h>
#include <iostream>

void setup_devices_subcommands(CLI::App& app){
    auto devs = app.add_subcommand("devices", "List connected input devices");
devs->callback([&]() {
    list_devices();
});
}

void list_devices() {
    UsbContext usb;

    libusb_device **list;
    ssize_t cnt = libusb_get_device_list(usb.get(), &list);
    if (cnt < 0) {
        throw std::runtime_error("Failed to get device list");
    }
    for (ssize_t i = 0; i < cnt; ++i) {
        libusb_device *dev = list[i];

        libusb_device_descriptor desc;
        if (libusb_get_device_descriptor(dev, &desc) < 0) {
            std::cerr << "Failed to get device descriptor" << std::endl;
            continue;
        }

        std::cout << "Device " << i
                  << ": VendorID=0x" << std::hex << desc.idVendor
                  << ", ProductID=0x" << std::hex << desc.idProduct
                  << std::dec << std::endl;
    }

    libusb_free_device_list(list, 1);
}
