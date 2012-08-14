#ifndef SONATAPLUGIN_H
#define SONATAPLUGIN_H

#include "abstractlyricsprovider.h"
#include <QtPlugin>
#include <QFile>
#include <QTextStream>

class SonataPlugin : public AbstractLyricsProvider
{
    Q_OBJECT
    Q_INTERFACES(AbstractLyricsProvider)

public:
    QString name() { return "Sonata"; }
    QString description() { return "/home/user/.lyrics/Sonata/"; }

    void fetch(QString artist, QString title);
    void abort() { };
};

#endif // SONATAPLUGIN_H
