#include "omplegacyplugin.h"

void OMPLegacyPlugin::fetch(QString artist, QString title)
{
    QFile file(QString("/home/user/.lyrics/%1-%2.txt").arg(cleanItem(artist), cleanItem(title)));

    file.open(QIODevice::ReadOnly | QIODevice::Text);
    QString lyrics = QTextStream(&file).readAll();
    file.close();

    if (lyrics.isEmpty())
        emit error("The lyrics for this song are missing from OMP legacy cache.");
    else
        emit fetched(lyrics);
}

QString OMPLegacyPlugin::cleanItem(QString item)
{
    return item.toLower().replace("&","and").remove(QRegExp("\\([^)]*\\)")).remove(QRegExp("[\\W_]"));
}

Q_EXPORT_PLUGIN2(omplegacyplugin, OMPLegacyPlugin)
