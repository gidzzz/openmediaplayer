#include <QtGui/QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("MohammadAG");
    QApplication::setApplicationName("mediaplayer");
    QApplication::setApplicationVersion("0.1");
    QApplication a(argc, argv);
    // TODO: Add a full list of contributors here when we're ready to release.
    qDebug() << "Open MediaPlayer, version:" << QApplication::applicationVersion()
             << "Running with PID:" << QApplication::applicationPid() << endl
             << "Copyright (C) 2010 Mohammad Abu-Garbeyyeh" << endl
             << "Licensed under GPLv3" << endl
             << "This program comes with ABSOLUTELY NO WARRANTY" << endl
             << "This is free software, and you are welcome to redistribute it" << endl
             << "under certain conditions; visit http://www.gnu.org/licenses/gpl.txt for details.";
    MainWindow w;

#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
