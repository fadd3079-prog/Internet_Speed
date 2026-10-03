#include "LinuxNetwork.h"

#include <ctime>
#include <fstream>
#include <sstream>

namespace
{
    constexpr const char* kProcNetRoute = "/proc/net/route";
}

LinuxNetworkMonitor::LinuxNetworkMonitor()
    : previousRx_(0)
    , previousTx_(0)
    , previousTimestampMs_(0)
    , currentSpeed_{0.0, 0.0}
    , initialized_(false)
{
}

bool LinuxNetworkMonitor::findDefaultInterface(std::string& interface) const
{
    std::ifstream route(kProcNetRoute);
    if (!route)
        return false;

    std::string line;
    std::getline(route, line); // skip header

    while (std::getline(route, line))
    {
        std::istringstream fields(line);
        std::string iface, destination;
        if (!(fields >> iface >> destination))
            continue;
        if (destination == "00000000")
        {
            interface = iface;
            return true;
        }
    }
    return false;
}

bool LinuxNetworkMonitor::readBytes(const std::string& interface,
                                    std::uint64_t& rxBytes,
                                    std::uint64_t& txBytes)
{
    const std::string base = "/sys/class/net/" + interface + "/statistics/";
    std::ifstream rxFile(base + "rx_bytes");
    std::ifstream txFile(base + "tx_bytes");
    if (!rxFile || !txFile)
        return false;
    rxFile >> rxBytes;
    txFile >> txBytes;
    return !rxFile.fail() && !txFile.fail();
}

std::uint64_t LinuxNetworkMonitor::nowMs()
{
    struct timespec ts{};
    clock_gettime(CLOCK_MONOTONIC, &ts);
    return static_cast<std::uint64_t>(ts.tv_sec) * 1000u +
           static_cast<std::uint64_t>(ts.tv_nsec) / 1000000u;
}

bool LinuxNetworkMonitor::update()
{
    std::string currentInterface;
    if (!findDefaultInterface(currentInterface))
    {
        currentSpeed_ = {0.0, 0.0};
        initialized_ = false;
        return false;
    }

    std::uint64_t rxBytes = 0;
    std::uint64_t txBytes = 0;
    if (!readBytes(currentInterface, rxBytes, txBytes))
    {
        currentSpeed_ = {0.0, 0.0};
        initialized_ = false;
        return false;
    }

    const std::uint64_t timestampMs = nowMs();

    if (!initialized_ || interface_ != currentInterface)
    {
        interface_ = currentInterface;
        previousRx_ = rxBytes;
        previousTx_ = txBytes;
        previousTimestampMs_ = timestampMs;
        currentSpeed_ = {0.0, 0.0};
        initialized_ = true;
        return true;
    }

    const std::uint64_t elapsedMs = timestampMs - previousTimestampMs_;
    if (elapsedMs == 0)
        return false;

    const std::uint64_t rxDelta = rxBytes >= previousRx_ ? rxBytes - previousRx_ : 0;
    const std::uint64_t txDelta = txBytes >= previousTx_ ? txBytes - previousTx_ : 0;
    const double elapsedSeconds = static_cast<double>(elapsedMs) / 1000.0;

    currentSpeed_.downloadMbps =
        static_cast<double>(rxDelta) * 8.0 / elapsedSeconds / 1000000.0;
    currentSpeed_.uploadMbps =
        static_cast<double>(txDelta) * 8.0 / elapsedSeconds / 1000000.0;

    previousRx_ = rxBytes;
    previousTx_ = txBytes;
    previousTimestampMs_ = timestampMs;
    return true;
}

NetworkSpeed LinuxNetworkMonitor::getSpeed() const
{
    return currentSpeed_;
}
