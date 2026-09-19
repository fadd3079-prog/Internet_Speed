#pragma once

#include <cstdint>

struct NetworkSpeed
{
    double downloadMbps;
    double uploadMbps;
};

class NetworkMonitor
{
public:
    NetworkMonitor();

    bool update();
    NetworkSpeed getSpeed() const;

private:
    std::uint64_t previousReceivedBytes_;
    std::uint64_t previousSentBytes_;
    std::uint64_t previousTimestampMs_;
    std::uint64_t interfaceLuid_;

    NetworkSpeed currentSpeed_;

    bool initialized_;
};