#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00

#include <windows.h>
#include <iphlpapi.h>

#include "Network.h"

namespace
{
    constexpr DWORD InternetAddress = 0x01010101;

    bool getActiveInterface(MIB_IFROW& row)
    {
        DWORD interfaceIndex = 0;

        if (GetBestInterface(
                InternetAddress,
                &interfaceIndex) != NO_ERROR)
        {
            return false;
        }

        MIB_IFROW candidate{};
        candidate.dwIndex = interfaceIndex;

        if (GetIfEntry(&candidate) != NO_ERROR)
            return false;

        if (candidate.dwOperStatus != MIB_IF_OPER_STATUS_OPERATIONAL)
            return false;

        if (candidate.dwType == IF_TYPE_SOFTWARE_LOOPBACK)
            return false;

        row = candidate;
        return true;
    }

    std::uint64_t calculateDelta(
        std::uint32_t current,
        std::uint32_t previous)
    {
        if (current >= previous)
            return current - previous;

        return (
            static_cast<std::uint64_t>(
                UINT32_MAX
            ) -
            previous +
            current +
            1
        );
    }
}

NetworkMonitor::NetworkMonitor()
    : interfaceIndex_(0),
      previousReceivedBytes_(0),
      previousSentBytes_(0),
      previousTimestampMs_(0),
      currentSpeed_{0.0, 0.0},
      initialized_(false)
{
}

bool NetworkMonitor::findInterface()
{
    MIB_IFROW row{};

    if (!getActiveInterface(row))
        return false;

    interfaceIndex_ = row.dwIndex;

    return true;
}

bool NetworkMonitor::update()
{
    MIB_IFROW row{};

    if (!getActiveInterface(row))
    {
        currentSpeed_ = {0.0, 0.0};
        initialized_ = false;
        interfaceIndex_ = 0;
        return false;
    }

    const std::uint64_t currentTimestampMs =
        GetTickCount64();

    const std::uint32_t currentInterfaceIndex =
        row.dwIndex;

    const std::uint32_t currentReceivedBytes =
        row.dwInOctets;

    const std::uint32_t currentSentBytes =
        row.dwOutOctets;

    if (!initialized_ ||
        interfaceIndex_ != currentInterfaceIndex)
    {
        interfaceIndex_ = currentInterfaceIndex;
        previousReceivedBytes_ = currentReceivedBytes;
        previousSentBytes_ = currentSentBytes;
        previousTimestampMs_ = currentTimestampMs;
        currentSpeed_ = {0.0, 0.0};
        initialized_ = true;
        return true;
    }

    const std::uint64_t elapsedMs =
        currentTimestampMs - previousTimestampMs_;

    if (elapsedMs == 0)
        return false;

    const std::uint64_t receivedDelta =
        calculateDelta(
            currentReceivedBytes,
            static_cast<std::uint32_t>(
                previousReceivedBytes_
            )
        );

    const std::uint64_t sentDelta =
        calculateDelta(
            currentSentBytes,
            static_cast<std::uint32_t>(
                previousSentBytes_
            )
        );

    const double elapsedSeconds =
        static_cast<double>(elapsedMs) / 1000.0;

    currentSpeed_.downloadMbps =
        static_cast<double>(receivedDelta) *
        8.0 /
        elapsedSeconds /
        1'000'000.0;

    currentSpeed_.uploadMbps =
        static_cast<double>(sentDelta) *
        8.0 /
        elapsedSeconds /
        1'000'000.0;

    previousReceivedBytes_ = currentReceivedBytes;
    previousSentBytes_ = currentSentBytes;
    previousTimestampMs_ = currentTimestampMs;

    return true;
}

NetworkSpeed NetworkMonitor::getSpeed() const
{
    return currentSpeed_;
}