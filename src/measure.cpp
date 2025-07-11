#include "measure.hpp"
#include <iostream>
#include <libevdev/libevdev.h>
#include <fcntl.h>
#include <unistd.h>
#include <chrono>
#include <thread>

void setup_measure_subcommands(CLI::App& app) {
    auto measure = app.add_subcommand("measure", "Measurement utilities");
    static int duration = 5;
    static bool verbose = false;

    auto latency = measure->add_subcommand("latency", "Measure input latency");
    latency->add_option("-t,--time", duration, "Duration in seconds");
    latency->add_flag("-v,--verbose", verbose, "Verbose output for each event");
    latency->callback([=]() { measure_latency(duration, verbose); });
}

void measure_latency(int duration, bool verbose){
    std::string event_path;
    std::cout << "Enter event device path (e.g. /dev/input/event5): ";
    std::cin >> event_path;

    int fd = open(event_path.c_str(), O_RDONLY|O_NONBLOCK);
    if (fd < 0) {
        perror("Failed to open event device");
        return;
    }

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

    std::vector<double> intervals_ms;

    while (std::chrono::steady_clock::now() < end_time) {
        struct input_event ev;
        rc = libevdev_next_event(dev, LIBEVDEV_READ_FLAG_NORMAL, &ev);
        // TODO: try catch shit when it fails?
        if (rc == 0) {
            if (ev.type == EV_KEY || ev.type == EV_REL || ev.type == EV_ABS) {
                auto now = std::chrono::steady_clock::now();
                if (first_event) {
                    first_event = false;
                } else {
                    auto interval_us = std::chrono::duration_cast<std::chrono::microseconds>(now - last_event_time).count();
                    double interval_ms = interval_us / 1000.0;
                    intervals_ms.push_back(interval_ms);

                    if (verbose) {
                        std::cout << "Event: type=" << ev.type << " code=" << ev.code 
                                  << " value=" << ev.value 
                                  << " | Interval: " << interval_ms << " ms" << std::endl;
                    }
                }
                last_event_time = now;
            }
        } else if (rc != -EAGAIN) {
            std::cerr << "Error reading event: " << strerror(-rc) << std::endl;
        }
        std::this_thread::sleep_for(std::chrono::milliseconds(1));
    }

    if (!intervals_ms.empty()) {
        double sum = std::accumulate(intervals_ms.begin(), intervals_ms.end(), 0.0);
        double avg = sum / intervals_ms.size();

        double sq_sum = std::inner_product(intervals_ms.begin(), intervals_ms.end(), intervals_ms.begin(), 0.0);
        double variance = (sq_sum / intervals_ms.size()) - (avg * avg);
        double stddev = std::sqrt(variance);

        std::cout << "\n=== Latency Summary ===" << std::endl;
        std::cout << "Samples: " << intervals_ms.size() << std::endl;
        std::cout << "Average interval: " << avg << " ms" << std::endl;
        std::cout << "Variance: " << variance << " ms^2" << std::endl;
        std::cout << "Standard deviation: " << stddev << " ms" << std::endl;
    } else {
        std::cout << "No events captured to calculate latency statistics." << std::endl;
    }

    libevdev_free(dev);
    close(fd);
}