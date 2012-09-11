#ifndef DARKLYRICSPLUGIN_H
#define DARKLYRICSPLUGIN_H

#include "abstractlyricsprovider.h"
#include <QtPlugin>
#include <QTextDocument>
#include <QRegExp>

class DarkLyricsPlugin : public AbstractLyricsProvider
{
    Q_OBJECT
    Q_INTERFACES(AbstractLyricsProvider)

public:
    DarkLyricsPlugin();

    QString name() { return "Dark Lyrics"; }
    QString description() { return "http://www.darklyrics.com"; }

    void fetch(QString artist, QString title);
    void abort();

private slots:
    void onArtistReplyReceived();
    void onAlbumReplyReceived();

private:
    QString title;
};

#endif // DARKLYRICSPLUGIN_H
