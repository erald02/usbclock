cmake_minimum_required(VERSION 3.10)
project(usbclock)

set(CMAKE_CXX_STANDARD 17)

find_package(CLI11 REQUIRED)
find_package(PkgConfig REQUIRED)
pkg_check_modules(LIBUSB REQUIRED libusb-1.0)
pkg_check_modules(LIBEVDEV REQUIRED libevdev)
pkg_check_modules(NCURSES REQUIRED ncurses)

include_directories(${LIBUSB_INCLUDE_DIRS})
include_directories(${LIBEVDEV_INCLUDE_DIRS})
include_directories(${NCURSES_INCLUDE_DIRS})

add_executable(usbclock
    src/main.cpp
    src/measure.cpp
    src/overclock.cpp
    src/devices.cpp
)

target_link_libraries(usbclock
    CLI11::CLI11
    ${LIBUSB_LIBRARIES}
    ${LIBEVDEV_LIBRARIES}
    ${NCURSES_LIBRARIES}
)
