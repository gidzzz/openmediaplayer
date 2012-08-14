#include "maelyricaplugin.h"

void MaeLyricaPlugin::fetch(QString artist, QString title)
{
    QTextDocument lyrics;
    lyrics.setHtml(QSettings("Marcin Mielniczuk", "MaeLyrica")
                   .value("lyrics/" + cleanItem(artist) + "-" + cleanItem(title))
                   .toString());

    if (lyrics.isEmpty())
        emit error("The lyrics for this song are missing from MaeLyrica cache.");
    else
        emit fetched(lyrics.toPlainText());
}

QString MaeLyricaPlugin::cleanItem(QString item)
{
    return item.toLower().replace("&","and").remove(QRegExp("\\([^)]*\\)")).remove(QRegExp("[\\W_]"));
}

Q_EXPORT_PLUGIN2(maelyricaplugin, MaeLyricaPlugin)
