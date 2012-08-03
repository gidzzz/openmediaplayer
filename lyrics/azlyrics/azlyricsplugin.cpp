#include "azlyricsplugin.h"

AZLyricsPlugin::AZLyricsPlugin()
{
    nam = new QNetworkAccessManager(this);
}

void AZLyricsPlugin::fetch(QString artist, QString title)
{
    artist = artist.toLower().remove(QRegExp("[^a-z0-9]"));
    title = title.toLower().remove(QRegExp("[^a-z0-9]"));

    QString url = QString("http://www.azlyrics.com/lyrics/%1/%2.html").arg(artist, title);

    reply = nam->get(QNetworkRequest(QUrl(url)));
    connect(reply, SIGNAL(finished()), this, SLOT(onReplyReceived()));
}

void AZLyricsPlugin::abort()
{
    reply->abort();
    reply->deleteLater();
}

void AZLyricsPlugin::onReplyReceived()
{
    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->errorString());
        reply->deleteLater();
        return;
    }

    QByteArray data = reply->readAll();
    reply->deleteLater();

    if (data.contains("<!-- start of lyrics -->\r\n")) {
        data.remove(0, data.indexOf("<!-- start of lyrics -->\r\n") + 26);
        data.remove(data.indexOf("\r\n<!-- end of lyrics -->"), data.length());

        QTextDocument lyrics;
        lyrics.setHtml(data);

        emit fetched(lyrics.toPlainText());
    } else {
        emit error("The lyrics for this song are missing on AZLyrics.");
    }
}

Q_EXPORT_PLUGIN2(azlyricsplugin, AZLyricsPlugin)
