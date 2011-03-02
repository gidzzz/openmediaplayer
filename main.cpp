/**************************************************************************
    Open MediaPlayer
    Copyright (C) 2010-2011 Mohammad Abu-Garbeyyeh
                            Nicolai Hess
                            Timur Kristof

    This program is free software: you can redistribute it and/or modify
    it under the terms of the GNU General Public License as published by
    the Free Software Foundation, either version 3 of the License, or
    (at your option) any later version.

    This program is distributed in the hope that it will be useful,
    but WITHOUT ANY WARRANTY; without even the implied warranty of
    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
    GNU General Public License for more details.

    You should have received a copy of the GNU General Public License
    along with this program.  If not, see <http://www.gnu.org/licenses/>.
**************************************************************************/

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
