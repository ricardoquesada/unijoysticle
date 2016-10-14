#include "mainwindow.h"
#include <QApplication>

int main(int argc, char *argv[])
{
    Q_INIT_RESOURCE(resources);

    QApplication app(argc, argv);

    // name code
    app.setOrganizationDomain(QLatin1String("retro.moe"));
    app.setApplicationName(QLatin1String("UniJoystiCle"));
    app.setApplicationVersion(QLatin1String(GIT_VERSION));

    app.setApplicationDisplayName(QLatin1String("UniJoystiCle Controller"));

#ifdef Q_OS_MAC
    app.setAttribute(Qt::AA_DontShowIconsInMenus);
#endif

#if QT_VERSION >= 0x050100
    // Enable support for highres images (added in Qt 5.1, but off by default)
    app.setAttribute(Qt::AA_UseHighDpiPixmaps);
#endif

    QApplication::setWindowIcon(QIcon(":/images/logo512.png"));

    MainWindow w;
    w.show();
    return app.exec();
}
