#ifndef MAFWUTILS_H
#define MAFWUTILS_H

#include <QString>
#include <glib.h>

namespace MafwUtils
{
    QString toQString(const GError *e);
}

#endif // MAFWUTILS_H
