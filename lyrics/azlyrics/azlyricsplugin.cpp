#include "azlyricsplugin.h"

AZLyricsPlugin::AZLyricsPlugin()
{
    nam = new QNetworkAccessManager(this);
}

void AZLyricsPlugin::fetch(QString artist, QString title)
{
    const QRegExp removePattern("[^a-z0-9]");
    artist = artist.toLower().remove(removePattern);
    title = title.toLower().remove(removePattern);

    QNetworkRequest request;
    request.setUrl(QString("http://www.azlyrics.com/lyrics/%1/%2.html").arg(artist, title));
    request.setRawHeader("User-Agent", USER_AGENT);

    reply = nam->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onReplyReceived()));
}

void AZLyricsPlugin::abort()
{
    disconnect(reply, SIGNAL(finished()), this, SLOT(onReplyReceived()));
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

    int i = data.indexOf("-->\r\n", data.indexOf("<!-- Usage"));
    if (i != -1) {
        i += 5;
        data.remove(data.indexOf("</div>", i), data.length());
        data.remove(0, i);

        QTextDocument lyrics;
        lyrics.setHtml(QString::fromUtf8(data));

        emit fetched(lyrics.toPlainText());
    } else {
        emit error("The lyrics for this song are missing on AZLyrics.");
    }
}

Q_EXPORT_PLUGIN2(azlyricsplugin, AZLyricsPlugin)
