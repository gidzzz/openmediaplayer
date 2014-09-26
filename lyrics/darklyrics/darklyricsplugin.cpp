#include "darklyricsplugin.h"

DarkLyricsPlugin::DarkLyricsPlugin()
{
    nam = new QNetworkAccessManager(this);
}

void DarkLyricsPlugin::fetch(QString artist, QString title)
{
    artist = artist.toLower().remove(QRegExp("[^a-z0-9]"));
    this->title = title;

    QNetworkRequest request;
    request.setUrl(QString("http://www.darklyrics.com/%1/%2.html").arg(artist.at(0), artist));
    request.setRawHeader("User-Agent", USER_AGENT);

    reply = nam->get(request);
    connect(reply, SIGNAL(finished()), this, SLOT(onArtistReplyReceived()));
}

void DarkLyricsPlugin::abort()
{
    disconnect(reply, SIGNAL(finished()), this, SLOT(onArtistReplyReceived()));
    disconnect(reply, SIGNAL(finished()), this, SLOT(onAlbumReplyReceived()));
    reply->abort();
    reply->deleteLater();
}

void DarkLyricsPlugin::onArtistReplyReceived()
{
    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->errorString());
        reply->deleteLater();
        return;
    }

    QString data = QString::fromUtf8(reply->readAll());
    reply->deleteLater();

    // <a href="../lyrics/anathema/anaturaldisaster.html#4">Are You There?</a><br />
    int i = data.indexOf(">" + title + "<", 0, Qt::CaseInsensitive);
    int j;
    if (i != -1) {
        j = data.lastIndexOf('#', i);
        i = data.lastIndexOf('"', j) + 3;

        QNetworkRequest request;
        request.setUrl("http://www.darklyrics.com" + data.mid(i, j-i));
        request.setRawHeader("User-Agent", USER_AGENT);

        reply = nam->get(request);
        connect(reply, SIGNAL(finished()), this, SLOT(onAlbumReplyReceived()));
    } else {
        emit error("The lyrics for this song are missing on Dark Lyrics.");
    }
}

void DarkLyricsPlugin::onAlbumReplyReceived()
{
    if (reply->error() != QNetworkReply::NoError) {
        emit error(reply->errorString());
        reply->deleteLater();
        return;
    }

    QString data = QString::fromUtf8(reply->readAll());
    reply->deleteLater();

    // <h3><a name="4">4. Are You There?</a></h3><br />
    // [lyrics]<br />
    // <br /><br />
    int i = data.indexOf(". " + title + "</a></h3>", 0, Qt::CaseInsensitive);
    int j;
    if (i != -1) {
        i = data.indexOf("<br />", i) + 6;
        j = data.indexOf("<br />\n<br /><br />", i);

        QTextDocument lyrics;
        lyrics.setHtml(data.mid(i, j-i));

        emit fetched(lyrics.toPlainText());
    } else {
        emit error("The lyrics for this song are missing on Dark Lyrics.");
    }
}

Q_EXPORT_PLUGIN2(darklyricsplugin, DarkLyricsPlugin)
