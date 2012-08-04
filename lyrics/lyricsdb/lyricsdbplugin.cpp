#include "lyricsdbplugin.h"

LyricsDBPlugin::LyricsDBPlugin()
{
    nam = new QNetworkAccessManager(this);
}

void LyricsDBPlugin::fetch(QString artist, QString title)
{
    QByteArray data;
    data.append("<?xml version=\"1.0\"?><query>");
    data.append(QString("<song id=\"0\" artist=\"%1\" title=\"%2\"/>").arg(Qt::escape(artist), Qt::escape(title)));
    data.append("</query>");

    reply = nam->post(QNetworkRequest(QUrl("http://lyrics.mirkforce.net/cgi-bin/lepserver.cgi")), data);
    connect(reply, SIGNAL(finished()), this, SLOT(onReplyReceived()));
}

void LyricsDBPlugin::abort()
{
    disconnect(reply, SIGNAL(finished()), this, SLOT(onReplyReceived()));
    reply->abort();
    reply->deleteLater();
}

void LyricsDBPlugin::onReplyReceived()
{
    QString lyrics;

    QXmlStreamReader xml(reply->readAll());
    reply->deleteLater();

    while (!xml.atEnd() && !xml.hasError())
        if (xml.readNext() == QXmlStreamReader::StartElement && xml.name() == "text")
            { lyrics = xml.readElementText(); break; }

    if (lyrics.isEmpty())
        emit error("The lyrics for this song are missing on LyricsDB.");
    else
        emit fetched(lyrics);


}

Q_EXPORT_PLUGIN2(lyricsdbplugin, LyricsDBPlugin)
