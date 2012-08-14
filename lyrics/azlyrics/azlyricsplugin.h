#ifndef AZLYRICSPLUGIN_H
#define AZLYRICSPLUGIN_H

#include "abstractlyricsprovider.h"
#include <QtPlugin>
#include <QTextDocument>
#include <QRegExp>

class AZLyricsPlugin : public AbstractLyricsProvider
{
    Q_OBJECT
    Q_INTERFACES(AbstractLyricsProvider)

public:
    AZLyricsPlugin();

    QString name() { return "AZLyrics"; }
    QString description() { return "http://www.azlyrics.com"; }

    void fetch(QString artist, QString title);
    void abort();

private slots:
    void onReplyReceived();
};

#endif // AZLYRICSPLUGIN_H
