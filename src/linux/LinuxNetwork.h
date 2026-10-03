#pragma once

#include <cstdint>
#include <string>

struct NetworkSpeed
{
    double downloadMbps;
    double uploadMbps;
};

class LinuxNetworkMonitor
{
public:
    LinuxNetworkMonitor();

    bool update();
    NetworkSpeed getSpeed() const;

private:
    bool findDefaultInterface(std::string& interface) const;
    static bool readBytes(const std::string& interface,
                          std::uint64_t& rxBytes,
                          std::uint64_t& txBytes);
    static std::uint64_t nowMs();

    std::string interface_;
    std::uint64_t previousRx_;
    std::uint64_t previousTx_;
    std::uint64_t previousTimestampMs_;

    NetworkSpeed currentSpeed_;
    bool initialized_;
};
