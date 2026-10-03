#include "TrayMeter.h"

#include <QApplication>
#include <QPainter>
#include <QPixmap>

namespace
{
    constexpr int kIconWidth = 144;
    constexpr int kIconHeight = 36;
    constexpr int kRefreshIntervalMs = 1000;
}

TrayMeter::TrayMeter(QObject* parent)
    : QSystemTrayIcon(parent)
    , exitAction_(new QAction(tr("Exit"), this))
{
    connect(exitAction_, &QAction::triggered, qApp, &QApplication::quit);
    menu_.addAction(exitAction_);
    setContextMenu(&menu_);

    connect(&timer_, &QTimer::timeout, this, &TrayMeter::refresh);
    timer_.start(kRefreshIntervalMs);
    refresh();
}

void TrayMeter::refresh()
{
    network_.update();
    const NetworkSpeed speed = network_.getSpeed();

    setIcon(renderIcon(speed.downloadMbps, speed.uploadMbps));
    setToolTip(QStringLiteral("Internet Speed\nDownload: %1 Mbps\nUpload: %2 Mbps")
                   .arg(speed.downloadMbps, 0, 'f', 1)
                   .arg(speed.uploadMbps, 0, 'f', 1));
}

QString TrayMeter::formatSpeed(double mbps)
{
    if (mbps >= 100.0)
        return QString::number(mbps, 'f', 0);
    return QString::number(mbps, 'f', 1);
}

QIcon TrayMeter::renderIcon(double downloadMbps, double uploadMbps) const
{
    QPixmap pixmap(kIconWidth, kIconHeight);
    pixmap.fill(Qt::transparent);

    QPainter painter(&pixmap);
    painter.setRenderHint(QPainter::Antialiasing);

    QFont font(QStringLiteral("Monospace"));
    font.setStyleHint(QFont::Monospace);
    font.setPointSize(20);
    font.setBold(true);
    painter.setFont(font);

    const QString text = QStringLiteral("↓%1 ↑%2")
                             .arg(formatSpeed(downloadMbps),
                                  formatSpeed(uploadMbps));

    // Dark outline so the text stays readable on light or dark tray bars.
    painter.setPen(QColor(0, 0, 0, 220));
    for (int dx = -1; dx <= 1; ++dx)
    {
        for (int dy = -1; dy <= 1; ++dy)
        {
            if (dx != 0 || dy != 0)
                painter.drawText(pixmap.rect(), Qt::AlignCenter, text);
        }
    }

    painter.setPen(Qt::white);
    painter.drawText(pixmap.rect(), Qt::AlignCenter, text);

    return QIcon(pixmap);
}
