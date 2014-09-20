#include "lyricwikiplugin.h"

LyricWikiPlugin::LyricWikiPlugin()
{
    nam = new QNetworkAccessManager(this);
}

void LyricWikiPlugin::fetch(QString artist, QString title)
{
    prepareName(artist);
    prepareName(title);

    QString url = QString("http://lyrics.wikia.com/%1:%2").arg(artist.replace(' ', '_'), title.replace(' ', '_'));

    reply = nam->get(QNetworkRequest(QUrl(url)));
    connect(reply, SIGNAL(finished()), this, SLOT(onReplyReceived()));
}

void LyricWikiPlugin::abort()
{
    disconnect(reply, SIGNAL(finished()), this, SLOT(onReplyReceived()));
    reply->abort();
    reply->deleteLater();
}

void LyricWikiPlugin::prepareName(QString &name)
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
}

void LyricWikiPlugin::onReplyReceived()
{
    QByteArray data = reply->readAll();
    reply->deleteLater();

    if (data.contains("<div class='lyricbox'>")) {
        data.remove(0, data.indexOf("<div class='lyricbox'>")+22);
        data.remove(data.indexOf("<!--")+4, data.length());

        if (data.contains("Category:Instrumental")) {
            emit error("According to LyricWiki this track is instrumental.");
            return;
        }

        data.remove(0, data.indexOf("</script>")+9);
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
