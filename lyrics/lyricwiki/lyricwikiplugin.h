#ifndef LYRICWIKIPLUGIN_H
#define LYRICWIKIPLUGIN_H

#include "abstractlyricsprovider.h"
#include <QtPlugin>
#include <QTextDocument>

class LyricWikiPlugin : public AbstractLyricsProvider
{
    Q_OBJECT
    Q_INTERFACES(AbstractLyricsProvider)

public:
    LyricWikiPlugin();

    QString name() { return "LyricWiki"; }

    void fetch(QString artist, QString title);
    void abort();

private slots:
    void onReplyReceived();
};

#endif // LYRICWIKIPLUGIN_H
