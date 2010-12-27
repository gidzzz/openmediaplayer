#include <QtGui/QApplication>

#include "mainwindow.h"

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("MohammadAG");
    QApplication::setApplicationName("mediaplayer");
    QApplication::setApplicationVersion("0.1");
    QApplication a(argc, argv);
    MainWindow w;

#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif

    return a.exec();
}
