#include "chartlyricsplugin.h"

ChartLyricsPlugin::ChartLyricsPlugin()
{
    nam = new QNetworkAccessManager(this);
}

void ChartLyricsPlugin::fetch(QString artist, QString title)
{
    QUrl url("http://api.chartlyrics.com/apiv1.asmx/SearchLyricDirect");
    url.addQueryItem("artist", artist);
    url.addQueryItem("song", title);

    reply = nam->get(QNetworkRequest(url));
    connect(reply, SIGNAL(finished()), this, SLOT(onReplyReceived()));
}

void ChartLyricsPlugin::abort()
{
    reply->abort();
    reply->deleteLater();
}

void ChartLyricsPlugin::onReplyReceived()
{
    QString lyrics;

    QXmlStreamReader xml(reply->readAll());
    reply->deleteLater();

    while (!xml.atEnd() && !xml.hasError())
        if (xml.readNext() == QXmlStreamReader::StartElement && xml.name() == "Lyric")
            { lyrics = xml.readElementText(); break; }

    if (lyrics.isEmpty())
        emit error("The lyrics for this song are missing on ChartLyrics.");
    else
        emit fetched(lyrics);
}

Q_EXPORT_PLUGIN2(chartlyricsplugin, ChartLyricsPlugin)
