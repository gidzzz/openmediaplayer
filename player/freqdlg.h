#ifndef FREQDLG_H
#define FREQDLG_H

#include <QtGui>
#include <QDialog>
#include <QListWidget>
#include <QFile>
#include <QDBusConnection>
#include <QDBusMessage>
#include <QDBusVariant>
#include <QDBusReply>

#include "ui_freqdlg.h"
#include "dbus/dbus-shared.h"
#include <libintl.h>
#include <gq/GConfItem>
#include "rotator.h"

/* /usr/include/gq is provided by libgq-gconf-dev and libgq-gconf0
   These packages need to be installed on top of your sysrootfs.
         Join #maemo-foss on FreeNode for more information. */

#define FMTX_SERVICE "com.nokia.FMTx"
#define FMTX_OBJ_PATH "/com/nokia/fmtx/default"
#define DBUS_PROPERTIES_SET "Set"
#define DBUS_PROPERTIES_GET "Get"

namespace Ui {
    class FreqDlg;
}

class FreqDlg : public QDialog
{
    Q_OBJECT
    int _minFreq;
    int _maxFreq;
    GConfItem *frequency;

public:
    QString res;
    explicit FreqDlg(QWidget *parent = 0);
    ~FreqDlg();
    QString currentValueText() const;
    QVariant getValue(QString property);
    QWidget *widget(QWidget *parent);
    double selectedFreq() const;
    void setSelectedFreq(double d);
    void setValue(QString property, QVariant value);
    void refreshFreqValues();

private slots:
    void on_buttonBox_accepted();
    void onFrequencyChanged();
    void onOrientationChanged(int w, int h);

private:
    Ui::FreqDlg *ui;
};

#endif // FREQDLG_H
