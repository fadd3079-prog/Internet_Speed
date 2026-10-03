#pragma once

#include <QAction>
#include <QMenu>
#include <QSystemTrayIcon>
#include <QTimer>

#include "LinuxNetwork.h"

class TrayMeter : public QSystemTrayIcon
{
    Q_OBJECT

public:
    explicit TrayMeter(QObject* parent = nullptr);

private slots:
    void refresh();

private:
    QIcon renderIcon(double downloadMbps, double uploadMbps) const;
    static QString formatSpeed(double mbps);

    QTimer timer_;
    QMenu menu_;
    QAction* exitAction_;
    LinuxNetworkMonitor network_;
};
