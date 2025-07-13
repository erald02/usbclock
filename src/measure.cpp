#include "measure.hpp"
#include <cstdint>
#include <iostream>
#include <libevdev/libevdev.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <thread>
#include <filesystem>
#include <vector>
#include <ncurses.h>


void setup_measure_subcommands(CLI::App& app) {
    auto measure = app.add_subcommand("measure", "Measurement utilities");
    static int duration = 5;
    static bool verbose = false;

    auto latency = measure->add_subcommand("latency", "Measure input latency");
    latency->add_option("-t,--time", duration, "Duration in seconds");
    latency->add_flag("-v,--verbose", verbose, "Verbose output for each event");
    latency->callback([&]() { measure_latency(duration, verbose); });
}

void measure_latency(int duration, bool verbose) {
    std::string event_path = select_device_interactively();
    if (event_path.empty()) {
        std::cout << "No device selected. Exiting." << std::endl;
        return;
    }

    int fd = open(event_path.c_str(), O_RDONLY | O_NONBLOCK);
    if (fd < 0) {
        perror("Failed to open event device");
        return;
    }


    std::cout << event_path << std::endl;

    struct libevdev *dev = nullptr;
    int rc = libevdev_new_from_fd(fd, &dev);
    if (rc < 0) {
        std::cerr << "Failed to init libevdev (" << strerror(-rc) << ")" << std::endl;
        close(fd);
        return;
    }

    std::cout << "Input device name: " << libevdev_get_name(dev) << std::endl;
    std::cout << "Measuring for " << duration << " seconds..." << std::endl;

    auto end_time = std::chrono::steady_clock::now() + std::chrono::seconds(duration);
    auto last_event_time = std::chrono::steady_clock::now();
    bool first_event = true;
    double mean = 0.0;
    int64_t reading = 0;
    double M2 = 0.0;

    while (std::chrono::steady_clock::now() < end_time) {
        struct input_event ev;
        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        if (rc == 0) {
            if (ev.type == EV_KEY || ev.type == EV_REL || ev.type == EV_ABS) {
                auto now = std::chrono::steady_clock::now();
                if (!first_event) {
                    auto interval = std::chrono::duration_cast<std::chrono::microseconds>(now - last_event_time).count();
                    ++reading;
                    double delta = interval - mean;
                    mean += delta / reading;
                    double delta2 = interval - mean;
                    M2 += delta * delta2;
                    if (verbose) {
                        std::cout << "Event: type=" << ev.type << " code=" << ev.code
                                  << " value=" << ev.value;
                        std::cout << " | Interval: " << interval << " us" << std::endl;
                    }
                }
                first_event = false;
                last_event_time = now;
            }
        } else if (rc != -EAGAIN) {
            std::cerr << "Error reading event: " << strerror(-rc) << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }
    std::cout << "\n=== Latency Summary ===" << std::endl;
    std::cout << "Samples: " << reading << std::endl;
    std::cout << "Average interval: " << (mean / 1000) << " ms" << std::endl;
    std::cout << "Variance: " << ((reading > 1 ? M2 / (reading - 1) : 0) / 1000000) << " ms^2" << std::endl;

    libevdev_free(dev);
    close(fd);
}

std::string select_device_interactively() {
    const std::string input_dir = "/dev/input/by-id/";
    std::vector<std::pair<std::string, std::string>> devices;

    namespace fs = std::filesystem;
    if (!fs::exists(input_dir) || !fs::is_directory(input_dir)) {
        std::cerr << "Directory " << input_dir << " not found.\n";
        return "";
    }

    for (const auto& entry : fs::directory_iterator(input_dir)) {
        if (fs::is_symlink(entry.symlink_status())) {
            const auto link_name = entry.path().filename().string();
            const auto target_path = fs::read_symlink(entry.path());
            const auto full_path = fs::absolute(entry.path().parent_path() / target_path).string();
            if (full_path.find("/event") != std::string::npos) {
                devices.emplace_back(link_name, full_path);
            }
        }
    }

    std::sort(devices.begin(), devices.end());

    if (devices.empty()) {
        std::cerr << "No input devices found in " << input_dir << "\n";
        return "";
    }

    initscr();
    noecho();
    cbreak();
    keypad(stdscr, TRUE);
    curs_set(0);

    int highlight = 0;
    int choice = -1;

    while (true) {
        clear();
        mvprintw(0, 0, "Select a device (↑↓ Enter to select, 'q' to quit):");
        for (size_t i = 0; i < devices.size(); ++i) {
            if (i == highlight) attron(A_REVERSE);
            mvprintw(i + 2, 2, "%s", devices[i].first.c_str());
            if (i == highlight) attroff(A_REVERSE);
        }
        refresh();

        int ch = getch();
        switch (ch) {
            case KEY_UP:    highlight = std::max(0, highlight - 1); break;
            case KEY_DOWN:  highlight = std::min((int)devices.size() - 1, highlight + 1); break;
            case 10:        choice = highlight; break; 
            case 'q':       endwin(); return "";       
        }
        if (choice != -1) break;
    }

    endwin();
    return devices[choice].second;
}
