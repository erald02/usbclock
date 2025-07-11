#include "devices.hpp"
#include <iostream>
#include <filesystem>
#include <libusb-1.0/libusb.h>
#include <iostream>

void setup_devices_subcommands(CLI::App& app){
    auto devs = app.add_subcommand("devices", "List connected input devices");
    devs->callback([&]() {
        auto devices = fetch_devices();
        print_devices(devices);
    });
}

std::vector<UsbDeviceInfo> fetch_devices() {
    UsbContext usb;
    std::vector<UsbDeviceInfo> devices;

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

        UsbDeviceInfo info;
        info.index = i;
        info.vendor_id = desc.idVendor;
        info.product_id = desc.idProduct;
        info.could_open = false;

        libusb_device_handle *handle;
        if (libusb_open(dev, &handle) == 0) {
            unsigned char manufacturer[256] = {0};
            unsigned char product[256] = {0};

            if (desc.iManufacturer > 0) {
                int r = libusb_get_string_descriptor_ascii(handle, desc.iManufacturer, manufacturer, sizeof(manufacturer));
                if (r >= 0) {
                    info.manufacturer = std::string(reinterpret_cast<char*>(manufacturer));
                }
            }
            if (desc.iProduct > 0) {
                int r = libusb_get_string_descriptor_ascii(handle, desc.iProduct, product, sizeof(product));
                if (r >= 0) {
                    info.product = std::string(reinterpret_cast<char*>(product));
                }
            }

            info.could_open = true;
            libusb_close(handle);
        }

        devices.push_back(info);
    }

    libusb_free_device_list(list, 1);

    return devices;
}

void print_devices(const std::vector<UsbDeviceInfo>& devices) {
    for (const auto& dev : devices) {
        std::cout << "Device " << dev.index
                  << ": VendorID=0x" << std::hex << dev.vendor_id
                  << ", ProductID=0x" << std::hex << dev.product_id;

        if (dev.could_open) {
            std::cout << ", Manufacturer: " << (dev.manufacturer.empty() ? "Unknown" : dev.manufacturer)
                      << ", Product: " << (dev.product.empty() ? "Unknown" : dev.product);
        } else {
            std::cout << ", Could not open device to retrieve names (try sudo)";
        }

        std::cout << std::dec << std::endl;
    }
}