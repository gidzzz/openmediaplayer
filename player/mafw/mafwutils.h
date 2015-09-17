#ifndef MAFWUTILS_H
#define MAFWUTILS_H

#include <QString>
#include <QVariant>
#include <glib-object.h>

namespace MafwUtils
{
    QString toQString(const GError *e);
    QVariant toQVariant(const GValue *v);
}

#endif // MAFWUTILS_H
