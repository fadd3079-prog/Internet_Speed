#include <QApplication>
#include <QSystemTrayIcon>

#include "TrayMeter.h"

int main(int argc, char* argv[])
{
    QApplication app(argc, argv);
    app.setQuitOnLastWindowClosed(false);
    app.setApplicationName(QStringLiteral("InternetSpeed"));
    app.setOrganizationName(QStringLiteral("Fadd Graphics"));

    if (!QSystemTrayIcon::isSystemTrayAvailable())
        return 1;

    TrayMeter tray;
    tray.show();

    return app.exec();
}
