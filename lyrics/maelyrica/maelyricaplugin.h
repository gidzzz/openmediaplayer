#ifndef MAELYRICAPLUGIN_H
#define MAELYRICAPLUGIN_H

#include "abstractlyricsprovider.h"
#include <QtPlugin>
#include <QTextDocument>
#include <QRegExp>
#include <QSettings>

class MaeLyricaPlugin : public AbstractLyricsProvider
{
    Q_OBJECT
    Q_INTERFACES(AbstractLyricsProvider)

public:
    QString name() { return "MaeLyrica"; }
    QString description() { return "http://maemo.org/packages/view/maelyrica/"; }

    void fetch(QString artist, QString title);
    void abort() { };

private:
    QString cleanItem(QString item);
};

#endif // MAELYRICAPLUGIN_H
