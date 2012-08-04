#include "lyricwikiplugin.h"

LyricWikiPlugin::LyricWikiPlugin()
{
    nam = new QNetworkAccessManager(this);
}

void LyricWikiPlugin::fetch(QString artist, QString title)
{
    QString url = QString("http://lyrics.wikia.com/%1:%2").arg(artist.replace(' ', '_'), title.replace(' ', '_'));

    reply = nam->get(QNetworkRequest(QUrl(url)));
    connect(reply, SIGNAL(finished()), this, SLOT(onReplyReceived()));
}

void LyricWikiPlugin::abort()
{
    reply->abort();
    reply->deleteLater();
}

void LyricWikiPlugin::onReplyReceived()
{
    QByteArray data = reply->readAll();
    reply->deleteLater();

    if (data.contains("<div class='lyricbox'>")) {
        data.remove(0, data.indexOf("<div class='lyricbox'>")+22);
        data.remove(data.indexOf("<!--")+4, data.length());

        data.remove(0, data.indexOf("</div>")+6);
        data.remove(data.indexOf("<!--"), 4);

        QTextDocument lyrics; lyrics.setHtml(data);
        QString plainLyrics = lyrics.toPlainText();

        if (plainLyrics.contains("we are not licensed to display the full lyrics for this song"))
            emit error("The lyrics for this song are incomplete on LyricWiki.");
        else
            emit fetched(plainLyrics);
    } else {
        emit error("The lyrics for this song are missing on LyricWiki.");
    }
}

Q_EXPORT_PLUGIN2(lyricwikiplugin, LyricWikiPlugin)
