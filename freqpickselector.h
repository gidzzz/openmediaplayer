#ifndef FREQPICKSELECTOR_H
#define FREQPICKSELECTOR_H

#include <QtGui>
#include <QMaemo5AbstractPickSelector>
#include <QFile>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QDBusReply>
#include "dbus/dbus-shared.h"
#include <libintl.h>
#include <gq/GConfItem>
/* /usr/include/gq is provided by libgq-gconf-dev and libgq-gconf0
   These packages need to be installed on top of your sysrootfs.
         Join #maemo-foss on FreeNode for more information. */

#define FMTX_SERVICE "com.nokia.FMTx"
#define FMTX_OBJ_PATH "/com/nokia/fmtx/default"
#define DBUS_PROPERTIES_SET "Set"
#define DBUS_PROPERTIES_GET "Get"

class FreqPickSelector : public QMaemo5AbstractPickSelector
{
    Q_OBJECT
    QDialog *freqDialog;
    QListWidget *integers;
    QListWidget *fractions;
    int _minFreq;
    int _maxFreq;
    GConfItem *frequency;

public:
    explicit FreqPickSelector(QObject *parent = 0);
    ~FreqPickSelector();

    QString currentValueText() const;
    QVariant getValue(QString property);
    QWidget *widget(QWidget *parent);
    double selectedFreq() const;
    void setSelectedFreq(double d);
    void refreshFreqValues();

private slots:
    void updateText();
    void onFrequencyChanged();
};

#endif // FREQPICKSELECTOR_H
