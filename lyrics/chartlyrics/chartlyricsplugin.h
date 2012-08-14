#ifndef CHARTLYRICSPLUGIN_H
#define CHARTLYRICSPLUGIN_H

#include "abstractlyricsprovider.h"
#include <QtPlugin>
#include <QXmlStreamReader>

class ChartLyricsPlugin : public AbstractLyricsProvider
{
    Q_OBJECT
    Q_INTERFACES(AbstractLyricsProvider)

public:
    ChartLyricsPlugin();

    QString name() { return "ChartLyrics"; }
    QString description() { return "http://www.chartlyrics.com"; }

    void fetch(QString artist, QString title);
    void abort();

private slots:
    void onReplyReceived();
};

#endif // CHARTLYRICSPLUGIN_H
