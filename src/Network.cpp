#define WIN32_LEAN_AND_MEAN
#define _WIN32_WINNT 0x0A00

#include <winsock2.h>
#include <windows.h>
#include <ws2ipdef.h>
#include <iphlpapi.h>

#include "Network.h"

namespace
{
    constexpr ULONG DestinationAddress = 0x01010101;

    bool getActiveInterface(MIB_IF_ROW2& row)
    {
        DWORD interfaceIndex = 0;

        if (GetBestInterface(
                DestinationAddress,
                &interfaceIndex) != NO_ERROR)
        {
            return false;
        }

        MIB_IF_ROW2 candidate{};
        candidate.InterfaceIndex = interfaceIndex;

        if (GetIfEntry2(&candidate) != NO_ERROR)
            return false;

        if (candidate.OperStatus != IfOperStatusUp)
            return false;

        if (candidate.Type == IF_TYPE_SOFTWARE_LOOPBACK)
            return false;

        row = candidate;
        return true;
    }
}

NetworkMonitor::NetworkMonitor()
    : previousReceivedBytes_(0),
      previousSentBytes_(0),
      previousTimestampMs_(0),
      interfaceLuid_(0),
      currentSpeed_{0.0, 0.0},
      initialized_(false)
{
}

bool NetworkMonitor::update()
{
    MIB_IF_ROW2 interfaceRow{};

    if (!getActiveInterface(interfaceRow))
        return false;

    const std::uint64_t currentInterfaceLuid =
        interfaceRow.InterfaceLuid.Value;

    const std::uint64_t receivedBytes =
        interfaceRow.InOctets;

    const std::uint64_t sentBytes =
        interfaceRow.OutOctets;

    const std::uint64_t timestampMs =
        GetTickCount64();

    if (!initialized_ ||
        interfaceLuid_ != currentInterfaceLuid)
    {
        previousReceivedBytes_ = receivedBytes;
        previousSentBytes_ = sentBytes;
        previousTimestampMs_ = timestampMs;
        interfaceLuid_ = currentInterfaceLuid;
        currentSpeed_ = {0.0, 0.0};
        initialized_ = true;
        return true;
    }

    const std::uint64_t elapsedMs =
        timestampMs - previousTimestampMs_;

    if (elapsedMs == 0)
        return false;

    if (receivedBytes < previousReceivedBytes_ ||
        sentBytes < previousSentBytes_)
    {
        previousReceivedBytes_ = receivedBytes;
        previousSentBytes_ = sentBytes;
        previousTimestampMs_ = timestampMs;
        currentSpeed_ = {0.0, 0.0};
        return true;
    }

    const std::uint64_t receivedDelta =
        receivedBytes - previousReceivedBytes_;

    const std::uint64_t sentDelta =
        sentBytes - previousSentBytes_;

    const double seconds =
        static_cast<double>(elapsedMs) / 1000.0;

    currentSpeed_.downloadMbps =
        static_cast<double>(receivedDelta) *
        8.0 /
        seconds /
        1'000'000.0;

    currentSpeed_.uploadMbps =
        static_cast<double>(sentDelta) *
        8.0 /
        seconds /
        1'000'000.0;

    previousReceivedBytes_ = receivedBytes;
    previousSentBytes_ = sentBytes;
    previousTimestampMs_ = timestampMs;

    return true;
}

NetworkSpeed NetworkMonitor::getSpeed() const
{
    return currentSpeed_;
}