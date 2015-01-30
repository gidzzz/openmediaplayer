#include "lyricwikiplugin.h"

LyricWikiPlugin::LyricWikiPlugin()
{
    nam = new QNetworkAccessManager(this);
}

void LyricWikiPlugin::fetch(QString artist, QString title)
{
    this->title = title;

    reply = nam->get(QNetworkRequest(QUrl("http://lyrics.wikia.com/" + prepareName(artist))));
    connect(reply, SIGNAL(finished()), this, SLOT(onArtistReplyReceived()));
}

void LyricWikiPlugin::abort()
{
    disconnect(reply, SIGNAL(finished()), this, SLOT(onArtistReplyReceived()));
    disconnect(reply, SIGNAL(finished()), this, SLOT(onSongReplyReceived()));
    reply->abort();
    reply->deleteLater();
}

QString& LyricWikiPlugin::prepareName(QString &name)
{
    // Capitalize
    const QChar delimiter = ' ';
    for (int i = name.length()-2; i >= 0; i--)
        if (name[i] == delimiter)
            name[i+1] = name[i+1].toUpper();
    name[0] = name[0].toUpper();

    // Sanitize
    name.replace(' ', '_')
        .replace('[', '(').replace(']', ')')
        .replace('{', '(').replace('}', ')')
        .replace('<', "Less_Than")
        .replace('>', "Greater_Than")
        .replace('#', "Number_");
    // NOTE: Depending on the context, "#" can also be replaced by "Sharp"
    // or omitted.

    return name;
}

void LyricWikiPlugin::onArtistReplyReceived()
{
    QByteArray data = reply->readAll();
    reply->deleteLater();

    if (!data.contains("<div class=\"noarticletext\">")) {
        // <link rel="canonical" href="http://lyrics.wikia.com/%D0%AD%D0%BF%D0%B8%D0%B4%D0%B5%D0%BC%D0%B8%D1%8F_(Epidemia)" />
        int i = data.indexOf("<link rel=\"canonical\"");
        if (i != -1) {
            i += 28;
            data.remove(data.indexOf('"', i), data.length());

            reply = nam->get(QNetworkRequest(QUrl::fromEncoded(data.mid(i) + ':' + QUrl::toPercentEncoding(prepareName(title)))));
            connect(reply, SIGNAL(finished()), this, SLOT(onSongReplyReceived()));
            return;
        }
    }

    emit error("The lyrics for this artist are missing on LyricWiki.");
}

void LyricWikiPlugin::onSongReplyReceived()
{
    QByteArray data = reply->readAll();
    reply->deleteLater();

    // <div class='lyricbox'>(...)<script>(...)</script>(lyrics)<!--
    int i = data.indexOf("<div class='lyricbox'>");
    if (i != -1) {
        i += 22;
        data.remove(data.indexOf("<!--", i), data.length());

        if (data.indexOf("Category:Instrumental", i) != -1) {
            emit error("According to LyricWiki this track is instrumental.");
            return;
        }

        QTextDocument lyrics; lyrics.setHtml(data.mid(data.indexOf("</script>", i) + 9));
        QString plainLyrics = lyrics.toPlainText();

        if (plainLyrics.contains("we are not licensed to display the full lyrics for this song")) {
            emit error("The lyrics for this song are incomplete on LyricWiki.");
        } else {
            emit fetched(plainLyrics);
        }
    } else {
        emit error("The lyrics for this song are missing on LyricWiki.");
    }
}

Q_EXPORT_PLUGIN2(lyricwikiplugin, LyricWikiPlugin)
