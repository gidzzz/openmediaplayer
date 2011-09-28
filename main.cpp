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
#include <QTime>
#include <QTextStream>
#include <QTranslator>

int main(int argc, char *argv[])
{
    QApplication::setOrganizationName("MohammadAG");
    QApplication::setApplicationName("mediaplayer");
    QApplication::setApplicationVersion("0.1");
    QApplication a(argc, argv);

    QString langPath = "/opt/openmediaplayer/";
    // This shit returns the device language, not the current language
    //QString lang = QLocale::languageToString(QLocale::system().language());

    // Open locale file to chech current language
    QString line, lang;
    QFile data( "/etc/osso-af-init/locale" );
    if (data.open(QFile::ReadOnly | QFile::Truncate))
    {
        QTextStream out(&data);
        while ( !out.atEnd() )
        {
            line = out.readLine();
            if ( line.indexOf("export LANG") == 0 )
            {
                lang = line;
                lang.replace("export LANG=","");
            }
        }
    }
    data.close();

    // Convert en_GB to English, es_ES to Spanish...
    QLocale::setDefault(lang);
    lang = QLocale::languageToString(QLocale(lang).language());

    // Install language file
    QTranslator translator;
    if (!lang.isEmpty() && !langPath.isEmpty())
    {
        if (QFile::exists(langPath + lang + ".qm"))
        {
            translator.load(langPath + lang);
            a.installTranslator(&translator);
        }
    }

    QTime t(0,0);
    t.start();
    // TODO: Add a full list of contributors here when we're ready to release.
    /*QTextStream out(stdout);
    out << "Open MediaPlayer, version: " << QApplication::applicationVersion() << " "
        << "Running with PID: " << QApplication::applicationPid() << endl
        << "Copyright (C) 2010-2011 Mohammad Abu-Garbeyyeh" << endl
        << "Licensed under GPLv3" << endl
        << "This program comes with ABSOLUTELY NO WARRANTY" << endl
        << "This is free software, and you are welcome to redistribute it" << endl
        << "under certain conditions; visit http://www.gnu.org/licenses/gpl.txt for details." << endl;*/
    MainWindow w;

    if (!QDBusConnection::sessionBus().isConnected()) {
        qWarning("Cannot connect to the D-Bus session bus.");
    }

#if defined(Q_WS_S60)
    w.showMaximized();
#else
    w.show();
#endif
    //out << QString("MediaPlayer startup took %1 ms").arg(t.elapsed()) << endl;
    return a.exec();
}
