#ifndef OMPLEGACYPLUGIN_H
#define OMPLEGACYPLUGIN_H

#include "abstractlyricsprovider.h"
#include <QtPlugin>
#include <QRegExp>
#include <QFile>
#include <QTextStream>

class OMPLegacyPlugin : public AbstractLyricsProvider
{
    Q_OBJECT
    Q_INTERFACES(AbstractLyricsProvider)

public:
    QString name() { return "OMP pre-20120803 offline cache"; }
    QString description() { return "/home/user/.lyrics/"; }

    void fetch(QString artist, QString title);
    void abort() { };

private:
    QString cleanItem(QString item);
};

#endif // OMPLEGACYPLUGIN_H
