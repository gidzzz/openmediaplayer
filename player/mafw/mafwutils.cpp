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

QVariant MafwUtils::toQVariant(const GValue *v)
{
    switch(G_VALUE_TYPE(v)) {
        case G_TYPE_BOOLEAN:
            return g_value_get_boolean(v);
        case G_TYPE_INT:
            return g_value_get_int(v);
        case G_TYPE_UINT:
            return g_value_get_uint(v);
        case G_TYPE_LONG:
            return (qlonglong) g_value_get_long(v);
        case G_TYPE_INT64:
            return g_value_get_int64(v);
        case G_TYPE_FLOAT:
            return g_value_get_float(v);
        case G_TYPE_DOUBLE:
            return g_value_get_double(v);
        case G_TYPE_STRING:
            return QString::fromUtf8(g_value_get_string(v));
        default:
            qDebug() << "Unsupported metadata type" << G_VALUE_TYPE_NAME(v);
            return QVariant();
    }
}
