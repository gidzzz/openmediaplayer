#include "mafwutils.h"

#include <QDebug>

QString MafwUtils::toQString(const GError *e)
{
    QString s;
    if (e) {
        s = e->message;
        qDebug() << s;
    }
    return s;
}
