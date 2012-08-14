#include "sonataplugin.h"

void SonataPlugin::fetch(QString artist, QString title)
{
    QFile file(QString("/home/user/.lyrics/Sonata/%1-%2.txt").arg(artist.remove('/'), title.remove('/')));

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString lyrics = QTextStream(&file).readAll();
    file.close();

    if (lyrics.isEmpty())
        emit error("The lyrics for this song are missing from Sonata cache.");
    else
        emit fetched(lyrics);
}

Q_EXPORT_PLUGIN2(sonataplugin, SonataPlugin)
